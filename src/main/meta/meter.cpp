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

#include <lsp-plug.in/plug-fw/meta/ports.h>
#include <lsp-plug.in/shared/meta/developers.h>
#include <private/meta/meter.h>

#define LSP_METER_VERSION_MAJOR       0
#define LSP_METER_VERSION_MINOR       0
#define LSP_METER_VERSION_MICRO       1

#define LSP_METER_VERSION  \
    LSP_MODULE_VERSION( \
        LSP_METER_VERSION_MAJOR, \
        LSP_METER_VERSION_MINOR, \
        LSP_METER_VERSION_MICRO  \
    )

namespace lsp
{
    namespace meta
    {
        //-------------------------------------------------------------------------
        // Meter
        static const int meter_classes[] = { C_ANALYSER, -1 };

        #define METER_INPUT(x, total) \
            AUDIO_INPUT_N(x), \
            AUDIO_OUTPUT_N(x), \
            { "on_" #x, "Analyse " #x, U_BOOL, R_CONTROL, F_IN, 0, 0, (x == 0) ? 1.0f : 0.0f, 0, NULL    }, \
            { "solo_" #x, "Solo " #x, U_BOOL, R_CONTROL, F_IN, 0, 0, 0, 0, NULL    }, \
            { "frz_" #x, "Freeze " #x, U_BOOL, R_CONTROL, F_IN, 0, 0, 0, 0, NULL    }, \
            { "hue_" #x, "Hue " #x, U_NONE, R_CONTROL, F_IN | F_UPPER | F_LOWER | F_STEP | F_CYCLIC, 0.0f, 1.0f, (float(x) / float(total)), 0.25f/360.0f, NULL     }, \
            AMP_GAIN("sh_" #x, "Shift gain " #x, 1.0f, 1000.0f), \
            MESH("ig_" #x, "Input graph" #x, 2, meter_metadata::HISTORY_MESH_SIZE)

        #define METER_COMMON() \
            BYPASS, \
            SWITCH("freeze", "Meter freeze", 0), \
            AMP_GAIN("pamp", "Preamp gain", meter_metadata::PREAMP_DFL, 1000.0f), \
            LOG_CONTROL("zoom", "Graph zoom", U_GAIN_AMP, meter_metadata::ZOOM), \
            { "time", "Time", U_SEC, R_CONTROL, F_IN | F_UPPER | F_LOWER | F_STEP, \
                 0.2f, 10.0f, 5.0f, 0.005f, NULL }

        static const port_t meter_x2_ports[] =
        {
            METER_INPUT(0, 2),
            METER_INPUT(1, 2),
            METER_COMMON(),
            
            PORTS_END
        };

        static const port_t meter_x16_ports[] =
        {
            METER_INPUT(0, 16),
            METER_INPUT(1, 16),
            METER_INPUT(2, 16),
            METER_INPUT(3, 16),
            METER_INPUT(4, 16),
            METER_INPUT(5, 16),
            METER_INPUT(6, 16),
            METER_INPUT(7, 16),
            METER_INPUT(8, 16),
            METER_INPUT(9, 16),
            METER_INPUT(10, 16),
            METER_INPUT(11, 16),
            METER_INPUT(12, 16),
            METER_INPUT(13, 16),
            METER_INPUT(14, 16),
            METER_INPUT(15, 16),
            METER_COMMON(),
            
            PORTS_END
        };

        #undef METER_INPUT
        #undef METER_COMMON

        const meta::bundle_t meter_bundle =
        {
            "meter",
            "Meter",
            B_ANALYZERS,
            "enrjenrjefnbv",
            "Meter plugin"
        };

        // Meter
        const meta::plugin_t  meter_x2 =
        {
            "Meter x2",
            "Meter x2",
            "ME2",
            &developers::v_sadovnikov,
            "meter_x2",
            LSP_LV2_URI("meter_x2"),
            LSP_LV2UI_URI("meter_x2"),
            "mex2",
            LSP_LADSPA_LIMITER_BASE + 0,
            LSP_LADSPA_URI("meter_x2"),
            LSP_METER_VERSION,
            meter_classes,
            E_INLINE_DISPLAY | E_DUMP_STATE,
            meter_x2_ports,
            "analyzer/meter/x2.xml",
            NULL,
            NULL,
            &meter_bundle
        };

        // Meter
        const meta::plugin_t  meter_x16 =
        {
            "Meter x16",
            "Meter x16",
            "ME16",
            &developers::v_sadovnikov,
            "meter_x16",
            LSP_LV2_URI("meter_x16"),
            LSP_LV2UI_URI("meter_x16"),
            "me16",
            LSP_LADSPA_LIMITER_BASE + 0,
            LSP_LADSPA_URI("meter_x16"),
            LSP_METER_VERSION,
            meter_classes,
            E_INLINE_DISPLAY | E_DUMP_STATE,
            meter_x16_ports,
            "analyzer/meter/x16.xml",
            NULL,
            NULL,
            &meter_bundle
        };
    } // namespace meta
} // namespace lsp
