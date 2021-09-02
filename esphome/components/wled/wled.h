void setPixelColor(uint16_t n, uint32_t c) {
  uint8_t w = (c >> 24);
  uint8_t r = (c >> 16);
  uint8_t g = (c >>  8);
  uint8_t b =  c       ;
  setPixelColor(n, r, g, b, w);
}

void setPixelColor(uint16_t i, byte r, byte g, byte b, byte w)
{
  //auto calculate white channel value if enabled
  if (isRgbw) {
    if (rgbwMode == RGBW_MODE_AUTO_BRIGHTER || (w == 0 && (rgbwMode == RGBW_MODE_DUAL || rgbwMode == RGBW_MODE_LEGACY)))
    {
      //white value is set to lowest RGB channel
      //thank you to @Def3nder!
      w = r < g ? (r < b ? r : b) : (g < b ? g : b);
    } else if (rgbwMode == RGBW_MODE_AUTO_ACCURATE && w == 0)
    {
      w = r < g ? (r < b ? r : b) : (g < b ? g : b);
      r -= w; g -= w; b -= w;
    }
  }
  
  if (SEGLEN) {//from segment

    //color_blend(getpixel, col, _bri_t); (pseudocode for future blending of segments)
    if (_bri_t < 255) {  
      r = scale8(r, _bri_t);
      g = scale8(g, _bri_t);
      b = scale8(b, _bri_t);
      w = scale8(w, _bri_t);
    }
    uint32_t col = ((w << 24) | (r << 16) | (g << 8) | (b));

    bool reversed = IS_REVERSE;
    uint16_t realIndex = realPixelIndex(i);
    uint16_t len = SEGMENT.length();

    for (uint16_t j = 0; j < SEGMENT.grouping; j++) {
      int indexSet = realIndex + (reversed ? -j : j);
      if (indexSet >= SEGMENT.start && indexSet < SEGMENT.stop) {
        if (IS_MIRROR) { //set the corresponding mirrored pixel
          uint16_t indexMir = SEGMENT.stop - indexSet + SEGMENT.start - 1;
          /* offset/phase */
          indexMir += SEGMENT.offset;
          if (indexMir >= SEGMENT.stop) indexMir -= len;

          if (indexMir < customMappingSize) indexMir = customMappingTable[indexMir];
          busses.setPixelColor(indexMir, col);
        }
        /* offset/phase */
          indexSet += SEGMENT.offset;
          if (indexSet >= SEGMENT.stop) indexSet -= len;

        if (indexSet < customMappingSize) indexSet = customMappingTable[indexSet];
        busses.setPixelColor(indexSet, col);
      }
    }
  } else { //live data, etc.
    if (i < customMappingSize) i = customMappingTable[i];
    
    uint32_t col = ((w << 24) | (r << 16) | (g << 8) | (b));
    busses.setPixelColor(i, col);
  }
}
