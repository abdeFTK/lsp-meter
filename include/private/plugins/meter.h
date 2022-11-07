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

#ifndef PRIVATE_PLUGINS_METER_H_
#define PRIVATE_PLUGINS_METER_H_

#include <lsp-plug.in/plug-fw/plug.h>
#include <lsp-plug.in/plug-fw/core/IDBuffer.h>
#include <lsp-plug.in/dsp-units/ctl/Bypass.h>
#include <lsp-plug.in/dsp-units/util/MeterGraph.h>
#include <lsp-plug.in/dsp-units/util/Oversampler.h>

#include <private/meta/meter.h>

namespace lsp
{
    namespace plugins
    {
        /**
         * Meter Plugin Series
         */
        class meter: public plug::Module
        {
            protected:

                typedef struct channel_t
                {
                    dspu::MeterGraph    sGraph;             // Input meter graph
                    bool                bOn;                // Enabled flag
                    bool                bFreeze;            // Freeze flag
                    bool                bSolo;              // Soloing flag
                    bool                bSend;              // Send to UI flag
                    float               fGain;              // Makeup gain
                    float               fHue;               // Hue
                    const float         *vIn;                // Input buffer pointer
                    float               *vOut;               // Output buffer pointer

                    float               *vDataBuf;           // Audio data buffer

                    // Port references
                    plug::IPort         *pIn;                // Input samples
                    plug::IPort         *pOut;               // Output samples
                    plug::IPort         *pOn;                // FFT on
                    plug::IPort         *pSolo;              // Soloing flag
                    plug::IPort         *pFreeze;            // Freeze flag
                    plug::IPort         *pHue;               // Hue of the graph color
                    plug::IPort         *pShift;             // Shift gain

                    plug::IPort         *pGraph;             // History graph
                } channel_t;

            protected:
                bool                create_channels(size_t channels);

            protected:
                size_t              nChannels;      // Number of channels
                channel_t          *vChannels;      // Audio channels
                float              *vTime;          // Time points buffer

                bool                bBypass;
                float               fPreamp;        // Preamplification level
                float               fZoom;          // Zoom
                float               fTime;          // Time

                bool                bUISync;        // Synchronize with UI

                plug::IPort        *pBypass;        // Bypass port
                plug::IPort        *pPreamp;
                plug::IPort        *pZoom;
                plug::IPort        *pFreeze;
                plug::IPort        *pTime;

                core::IDBuffer     *pIDisplay;      // Inline display buffer

            public:
                explicit meter(const meta::plugin_t *metadata);
                virtual ~meter();

            public:
                virtual void        init(plug::IWrapper *wrapper, plug::IPort **ports);
                virtual void        destroy();

                virtual void        update_settings();
                virtual void        update_sample_rate(long sr);
                virtual void        ui_activated();

                virtual void        process(size_t samples);
                virtual bool        inline_display(plug::ICanvas *cv, size_t width, size_t height);

                virtual void        dump(dspu::IStateDumper *v) const;
        };
    } // namespace plugins
} // namespace lsp

#endif /* PRIVATE_PLUGINS_LIMITER_H_ */
