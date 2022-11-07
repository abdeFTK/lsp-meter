/*
 * Copyright (C) 2021 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2021 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins-limiter
 * Created on: 3 авг. 2021 г.
 *
 * lsp-plugins-limiter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-plugins-limiter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-plugins-limiter. If not, see <https://www.gnu.org/licenses/>.
 */

#include <private/plugins/meter.h>
#include <lsp-plug.in/common/alloc.h>
#include <lsp-plug.in/dsp/dsp.h>
#include <lsp-plug.in/dsp-units/units.h>
#include <lsp-plug.in/plug-fw/meta/ports.h>
#include <lsp-plug.in/plug-fw/meta/func.h>
#include <lsp-plug.in/stdlib/math.h>
#include <lsp-plug.in/shared/id_colors.h>

#define LIMIT_BUFSIZE           8192
#define LIMIT_BUFMULTIPLE       16
#define TRACE_PORT(p)           lsp_trace("  port id=%s", (p)->metadata()->id);

namespace lsp
{
    namespace plugins
    {
        //-------------------------------------------------------------------------
        // Plugin factory
        static const meta::plugin_t *plugins[] =
        {
            &meta::meter_x2,
            &meta::meter_x16
        };

        static plug::Module *plugin_factory(const meta::plugin_t *meta)
        {
            return new meter(meta);
        }

        static plug::Factory factory(plugin_factory, plugins, 2);

        //-------------------------------------------------------------------------
        meter::meter(const meta::plugin_t *metadata): plug::Module(metadata)
        {
            nChannels       = 0;
            vChannels       = NULL;
            vTime           = NULL;

            bBypass         = false;
            fPreamp         = 0.0f;
            fZoom           = 0.0f;
            bUISync         = true;
            fTime           = 5.0f;

            pBypass         = NULL;
            pPreamp         = NULL;
            pZoom           = NULL;
            pFreeze         = NULL;
            pTime           = NULL;

            pIDisplay       = NULL;
        }

        meter::~meter()
        {
            vChannels       = NULL;
            pIDisplay       = NULL;
            vTime           = NULL;
        }

        bool meter::create_channels(size_t channels)
        {
            lsp_trace("this=%p, channels = %d", this, int(channels));

            lsp_trace("time_data_size    = %d", int(meta::meter_metadata::HISTORY_MESH_SIZE));

            // Initialize core
            nChannels       = channels;
            fPreamp         = meta::meter_metadata::PREAMP_DFL;
            fTime           = 5.0f;

            // Initialize pointers and cleanup buffers
            vTime           = new float[meta::meter_metadata::HISTORY_MESH_SIZE];
            
            // Allocate channels
            vChannels       = new channel_t[nChannels];
            
            lsp_trace("vChannels = %p", vChannels);

            // Initialize channels
            for (size_t i=0; i<channels; ++i)
            {
                channel_t *c     = &vChannels[i];

                // Initialize fields
                c->bOn              = false;
                c->bSolo            = false;
                c->bSend            = false;
                c->fGain            = 1.0f;
                c->fHue             = 0.0f;
                c->vIn              = NULL;
                c->vOut             = NULL;

                // Port references
                c->pIn              = NULL;
                c->pOut             = NULL;
                c->pOn              = NULL;
                c->pFreeze          = NULL;
                c->pHue             = NULL;
                c->pShift           = NULL;
                c->pGraph           = NULL;

                c->vDataBuf         = new float[LIMIT_BUFSIZE];
            }

            return true;
        }

        void meter::init(plug::IWrapper *wrapper, plug::IPort **ports)
        {
            plug::Module::init(wrapper, ports);

            // Determine number of channels
            size_t channels     = 0;
            if (pMetadata == NULL)
                return;
            for (const meta::port_t *p=pMetadata->ports; p->id != NULL; ++p)
                if (meta::is_audio_port(p) && (meta::is_in_port(p)))
                    ++channels;

            // Allocate channels
            if (!create_channels(channels))
                return;

            // Seek for first input port
            size_t port_id = 0;

            // Now we are available to map the ports for channels
            for (size_t i=0; i<nChannels; ++i)
            {
                lsp_trace("binding channel %d", int(i));

                plug::IPort *vp = ports[port_id];
                if (vp == NULL)
                    break;
                const meta::port_t *p = vp->metadata();
                if (p == NULL)
                    break;
                if ((p->id == NULL) || (!meta::is_audio_port(p)) || (!meta::is_in_port(p)))
                    break;

                channel_t *c        = &vChannels[i];
                c->pIn              = ports[port_id++];
                c->pOut             = ports[port_id++];
                c->pOn              = ports[port_id++];
                c->pSolo            = ports[port_id++];
                c->pFreeze          = ports[port_id++];
                c->pHue             = ports[port_id++];
                c->pShift           = ports[port_id++];
                c->pGraph           = ports[port_id++];

                // Sync metadata
                const meta::port_t *meta  = c->pSolo->metadata();
                if (meta != NULL)
                    c->bSolo        = meta->start >= 0.5f;

                meta                = c->pShift->metadata();
                if (meta != NULL)
                    c->fGain            = meta->start;

                lsp_trace("channel %d successful bound", int(i));
            }

            // Initialize basic ports
            pBypass         = ports[port_id++];
            pFreeze         = ports[port_id++];
            pPreamp         = ports[port_id++];
            pZoom           = ports[port_id++];
            pTime           = ports[port_id++];

            float delta     = meta::meter_metadata::HISTORY_TIME / (meta::meter_metadata::HISTORY_MESH_SIZE - 1);
            for (size_t i=0; i<meta::meter_metadata::HISTORY_MESH_SIZE; ++i)
                vTime[i]    = meta::meter_metadata::HISTORY_TIME - i*delta;

            lsp_trace("this=%p, basic ports successful bound", this);
        }

        void meter::destroy()
        {
            if (vTime != NULL)
            {
                delete [] vTime;
                vTime       = NULL;
            }
            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c     = &vChannels[i];
                if (c->vDataBuf != NULL) {
                    delete[] c->vDataBuf;
                    c->vDataBuf = NULL;
                }
            }
            if (vChannels != NULL)
            {
                delete [] vChannels;
                vChannels       = NULL;
            }
            if (pIDisplay != NULL)
            {
                pIDisplay->destroy();
                pIDisplay = NULL;
            }
        }

        void meter::update_sample_rate(long sr)
        {
            size_t real_sample_rate     = sr;
            //float scaling_factor        = meta::meter_metadata::HISTORY_TIME / meta::meter_metadata::HISTORY_MESH_SIZE;
            float scaling_factor          = fTime / meta::meter_metadata::HISTORY_MESH_SIZE;

            size_t real_samples_per_dot = dspu::seconds_to_samples(real_sample_rate, scaling_factor);

            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c = &vChannels[i];

                c->sGraph.init(meta::meter_metadata::HISTORY_MESH_SIZE, real_samples_per_dot);
                c->sGraph.set_period(real_samples_per_dot);
            }
        }

        void meter::update_settings()
        {
            // Update global settings
            bBypass                 = pBypass->value();
            fPreamp                 = pPreamp->value();
            fZoom                   = pZoom->value();
            fTime                   = pTime->value();

            float historyTime = fTime;

            float scaling_factor          = historyTime / meta::meter_metadata::HISTORY_MESH_SIZE;
            size_t real_samples_per_dot = dspu::seconds_to_samples(fSampleRate, scaling_factor);
            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c = &vChannels[i];
                c->sGraph.set_period(real_samples_per_dot);
            }

            float delta     = historyTime / (meta::meter_metadata::HISTORY_MESH_SIZE - 1);
            for (size_t i=0; i<meta::meter_metadata::HISTORY_MESH_SIZE; ++i)
                vTime[i]    = historyTime - i*delta;

            lsp_trace("preamp       = %.3f",   fPreamp);

            // Check that there are soloing channels
            size_t has_solo         = 0;
            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c     = &vChannels[i];
                if (c->pSolo->value() >= 0.5f)
                    has_solo++;
            }

            // Process channel parameters
            bool freeze_all     = pFreeze->value() >= 0.5f;

            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c       = &vChannels[i];

                c->bOn              = c->pOn->value() >= 0.5f;
                c->bFreeze          = (freeze_all) || (c->pFreeze->value() >= 0.5f);
                c->bSolo            = c->pSolo->value() >= 0.5f;
                c->bSend            = (c->bOn) && ((has_solo == 0) || ((has_solo > 0) && (c->bSolo)));
                c->fGain            = c->pShift->value();
                c->fHue             = c->pHue->value();
            }
        }

        void meter::process(size_t samples)
        {
            //pWrapper->query_display_draw();
            
            // Bind audio ports
            for (size_t i=0; i<nChannels; ++i)
            {
                channel_t *c    = &vChannels[i];
                c->vIn          = c->pIn->buffer<float>();
                c->vOut         = c->pOut->buffer<float>();
            }

            size_t buf_size     = LIMIT_BUFSIZE;

            // Process samples
            for (size_t nsamples = samples; nsamples > 0; )
            {
                size_t to_do    = lsp_min(buf_size, nsamples);

                for (size_t i=0; i<nChannels; ++i)
                {
                    channel_t *c    = &vChannels[i];

                    // Always bypass signal
                    dsp::copy(c->vOut, c->vIn, to_do);
                    
                    // Apply gain if needed
                    if (c->fGain != GAIN_AMP_0_DB || fPreamp != GAIN_AMP_0_DB)
                    {
                        dsp::mul_k3(c->vDataBuf, c->vIn, c->fGain * fPreamp, to_do);
                    } else 
                    {
                        dsp::copy(c->vDataBuf, c->vIn, to_do);
                    }

                    if (!c->bFreeze) {
                        // Update graphs
                        c->sGraph.process(c->vDataBuf, to_do);
                    }

                    // Update pointers
                    c->vIn         += to_do;
                    c->vOut        += to_do;
                }

                // Decrement number of samples for processing
                nsamples   -= to_do;
            }

            // Process mesh requests
            for (size_t i=0; i<nChannels; ++i)
            {
                // Get channel
                channel_t *c        = &vChannels[i];

                if (!c->bSend) {
                    dsp::fill_zero(c->sGraph.data(), meta::meter_metadata::HISTORY_MESH_SIZE);
                }

                // Get mesh
                plug::mesh_t *mesh    = c->pGraph->buffer<plug::mesh_t>();
                if ((mesh != NULL) && (mesh->isEmpty()))
                {
                    // Fill mesh with new values
                    dsp::copy(mesh->pvData[0], vTime, meta::meter_metadata::HISTORY_MESH_SIZE);
                    dsp::copy(mesh->pvData[1], c->sGraph.data(), meta::meter_metadata::HISTORY_MESH_SIZE);
                    mesh->data(2, meta::meter_metadata::HISTORY_MESH_SIZE);
                }
            }

            // Request for redraw
            if (pWrapper != NULL)
               pWrapper->query_display_draw();
        }

        void meter::ui_activated()
        {
            bUISync = true;
        }

        bool meter::inline_display(plug::ICanvas *cv, size_t width, size_t height)
        {
            return false;
        }

        void meter::dump(dspu::IStateDumper *v) const
        {
            plug::Module::dump(v);

            v->write("nChannels", nChannels);
        }
    } // namespace plugins
} // namespace lsp


