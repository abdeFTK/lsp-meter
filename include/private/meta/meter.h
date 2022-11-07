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

#ifndef PRIVATE_META_METER_H_
#define PRIVATE_META_METER_H_

#include <lsp-plug.in/plug-fw/meta/types.h>
#include <lsp-plug.in/plug-fw/const.h>


namespace lsp
{
    namespace meta
    {
        struct meter_metadata
        {
            static constexpr float          HISTORY_TIME            = 5.0f;     // Amount of time to display history [s]
            static constexpr size_t         HISTORY_MESH_SIZE       = 560;      // 420 dots for history

            static constexpr float          ZOOM_MIN                = GAIN_AMP_M_36_DB;
            static constexpr float          ZOOM_MAX                = GAIN_AMP_0_DB;
            static constexpr float          ZOOM_DFL                = GAIN_AMP_0_DB;
            static constexpr float          ZOOM_STEP               = 0.025f;

            static constexpr float          PREAMP_DFL              = 1.0;
        };

        extern const meta::plugin_t meter_x2;
        extern const meta::plugin_t meter_x16;
    } // namespace meta
} // namespace lsp


#endif /* PRIVATE_META_METER_H_ */
