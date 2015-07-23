// Copyright (C) 2015 Tomasz Miąsko
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
#include "headers_generator.hpp"

#include <glog/logging.h>
#include <vorbis/vorbisenc.h>

namespace fsb { namespace vorbis {

headers_generator::headers_generator(int channels, int rate, int quality) {
  CHECK(1 <= quality && quality <= 100)
    << "Quality should be in range [1, 100]: " << quality;
  
  // FSB quality is integer in range [1, 100], 
  // Vorbis quality is float in range [-0.1, 1].
  // Use linear interpolation to convert between them.
  const float vorbis_quality = ((quality - 1) + (quality - 100) * 0.1) / 99.0;

  int ret = vorbis_encode_setup_vbr(info_, channels, rate, vorbis_quality);
  CHECK(ret == 0) << "vorbis_encode_setup_vbr failed: " << ret;

  int arg = 1;
  ret = vorbis_encode_ctl(info_, OV_ECTL_COUPLING_SET, &arg);
  CHECK(ret == 0) << "vorbis_encode_ctl failed: " << ret;

  ret = vorbis_encode_setup_init(info_);
  CHECK(ret == 0) << "vorbis_encode_setup_init failed: " << ret;

  ret = vorbis_analysis_init(&dsp_state_, info_);
  CHECK(ret == 0) << "vorbis_analysis_init failed: " << ret;

  ret = vorbis_analysis_headerout(
    &dsp_state_, comment_,
    &header_id_, &header_comment_, &header_setup_);
  CHECK(ret == 0) << "vorbis_analysis_headerout failed:" << ret;
}

headers_generator::~headers_generator() {
  vorbis_dsp_clear(&dsp_state_);
}

}}
