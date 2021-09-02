//Segment option byte bits
#define SEG_OPTION_REVERSED       1
#define SEG_OPTION_TRANSITIONAL   7

//Segment differs return byte
#define SEG_DIFFERS_BRI        0x01
#define SEG_DIFFERS_FX         0x08
#define SEG_DIFFERS_OPT        0x02
#define SEG_DIFFERS_COL        0x04
#define SEG_DIFFERS_BOUNDS     0x10
#define SEG_DIFFERS_GSO        0x20

//RGB to RGBW conversion mode
#define RGBW_MODE_DUAL            3            //Manual slider + auto calculation. Automatically calculates only if manual slider is set to off (0)