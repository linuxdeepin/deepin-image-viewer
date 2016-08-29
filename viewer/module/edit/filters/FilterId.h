#ifndef FILTERID_H
#define FILTERID_H

namespace filter2d {
// ids can be used in filters, and filter components
enum {
    // samplers
    kSpread,
    kMediumBlur,
    kCarving,
    kRelief,
    kEdgeDetcet,

    //color transforms
    kSepia,
    kFreezing,
    kComic,
    kCasting,
    kInstagram1977,
    kCold,
    kBrightness,
    kContrast,
    kSaturation,
    kHue,
    kEQ,
    kEnd
};
} //namespace filter2d
#endif // FILTERID_H
