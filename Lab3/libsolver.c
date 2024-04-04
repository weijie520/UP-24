#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
#include "libmaze.h"

typedef void (*funcPtr)(maze_t* mz);
// #define MAIN_REAL 0x55d5eade17a9
#define MAIN_OFFSET 0x1b7a9
// unsigned long int main_offset = 0x1b7a9;
// #define MAIN_OFFSET 0x16c69
void* addr;
// static void * __stored_ptr = NULL;
static int is_visited[_MAZE_MAXY][_MAZE_MAXX] = {0};
static long int func_offset[1200] = {0x23508, 0x226c8, 0x22bb8, 0x21ce0, 0x22168, 0x23988, 0x23e18, 0x23020, 0x234d8, 0x23760, 0x232b0, 0x22e20, 0x22960, 0x22390, 0x21f98, 0x21b18, 0x23c20, 0x23790, 0x232d8, 0x23d40, 0x237d0, 0x233e8, 0x22f40, 0x22a98, 0x225c8, 0x220a8, 0x21c50, 0x23d50, 0x237f0, 0x23038, 0x22ba0, 0x225f8, 0x221b0, 0x21d38, 0x23e48, 0x239b8, 0x234e8, 0x23058, 0x22bb0, 0x22cf0, 0x22808, 0x22310, 0x21e78, 0x23f60, 0x23ad0, 0x235e0, 0x23130, 0x22cc0, 0x227b8, 0x23c18, 0x23780, 0x232f0, 0x22e40, 0x22970, 0x22428, 0x21f58, 0x21af0, 0x23bd8, 0x23758, 0x23878, 0x233f8, 0x22f48, 0x22a88, 0x225b0, 0x220c8, 0x21c38, 0x23d38, 0x23868, 0x233b8, 0x221c8, 0x21d40, 0x23e40, 0x239a0, 0x23500, 0x23040, 0x22b98, 0x22690, 0x22180, 0x21d18, 0x23ac8, 0x235d8, 0x23128, 0x22cb8, 0x227b0, 0x222e8, 0x21e50, 0x23f48, 0x23ac0, 0x23538, 0x22e38, 0x22968, 0x22420, 0x21f50, 0x21ae8, 0x23bd0, 0x23750, 0x232a8, 0x22e18, 0x22958, 0x229b0, 0x22480, 0x21fc8, 0x21b48, 0x23c48, 0x23690, 0x231e8, 0x22d68, 0x22878, 0x22370, 0x21c90, 0x23d78, 0x238b0, 0x23418, 0x22f60, 0x22ab8, 0x224c8, 0x22008, 0x21b68, 0x23c80, 0x23088, 0x22bf0, 0x22700, 0x221f8, 0x21d88, 0x23e70, 0x239d8, 0x23458, 0x22fa8, 0x22b00, 0x22380, 0x21eb0, 0x23fa8, 0x23b08, 0x23648, 0x231a0, 0x22d18, 0x22818, 0x22228, 0x21dd0, 0x23730, 0x232a0, 0x22e00, 0x22930, 0x22410, 0x21f40, 0x21ac0, 0x23bc0, 0x23708, 0x23270, 0x22080, 0x21c20, 0x23d18, 0x23858, 0x233a8, 0x22ef0, 0x22a40, 0x22548, 0x22060, 0x21bf0, 0x22b70, 0x22680, 0x22150, 0x21ce8, 0x23dc8, 0x23958, 0x234b0, 0x23010, 0x22b58, 0x22650, 0x23a98, 0x235b8, 0x23108, 0x22c78, 0x22788, 0x222a8, 0x21e28, 0x23f10, 0x23a68, 0x23598, 0x21f38, 0x21ab8, 0x23bb8, 0x23700, 0x23268, 0x22dd8, 0x22900, 0x223f0, 0x21f10, 0x21a88, 0x23850, 0x233a0, 0x22ee8, 0x22a38, 0x22540, 0x22058, 0x21be8, 0x23cf0, 0x23838, 0x23380, 0x22a90, 0x22f38, 0x233d8, 0x23880, 0x23d30, 0x21c30, 0x220a0, 0x22590, 0x22980, 0x22e48, 0x23b80, 0x21a70, 0x21ef8, 0x223d8, 0x228c8, 0x22da0, 0x23218, 0x236d0, 0x23b40, 0x23fd0, 0x22c40, 0x230c8, 0x23570, 0x23a40, 0x23ed8, 0x21de8, 0x22270, 0x22730, 0x22c18, 0x230a0, 0x22130, 0x22638, 0x22b30, 0x22fd8, 0x23478, 0x23908, 0x23d98, 0x21ca0, 0x22108, 0x22618, 0x23820, 0x23cb8, 0x21bb8, 0x22030, 0x22508, 0x229e0, 0x22e98, 0x23330, 0x237e8, 0x23cb0, 0x22d98, 0x23210, 0x236c8, 0x23b38, 0x23fc8, 0x21ec0, 0x223a0, 0x228c0, 0x22d88, 0x231f0, 0x23a38, 0x23ed0, 0x21de0, 0x22268, 0x22728, 0x22c10, 0x23098, 0x23540, 0x23a30, 0x23ec8, 0x23900, 0x23d90, 0x21c98, 0x22100, 0x22610, 0x22b28, 0x22fc8, 0x23468, 0x238e0, 0x23e00, 0x22028, 0x22500, 0x229d8, 0x22e90, 0x23328, 0x237e0, 0x23ca8, 0x21ba0, 0x22018, 0x224d8, 0x21e70, 0x22338, 0x22848, 0x22d28, 0x23180, 0x23630, 0x23b78, 0x21a68, 0x21ef0, 0x223d0, 0x22a50, 0x23848, 0x23390, 0x21c08, 0x23d08, 0x22560, 0x22070, 0x22df0, 0x22928, 0x23720, 0x21cf8, 0x23de0, 0x22678, 0x22148, 0x23018, 0x22b68, 0x23950, 0x234b8, 0x21c18, 0x23d10, 0x22c08, 0x22718, 0x23530, 0x23090, 0x23e80, 0x239f0, 0x22200, 0x21da0, 0x22bd0, 0x226d8, 0x23b30, 0x22388, 0x21eb8, 0x22d38, 0x22850, 0x23650, 0x231b0, 0x23f80, 0x23ae8, 0x22330, 0x237c8, 0x23320, 0x21b58, 0x23c60, 0x22498, 0x21fd8, 0x22e60, 0x22990, 0x237a0, 0x232e8, 0x220f8, 0x22f78, 0x22ad8, 0x238c8, 0x23428, 0x21c60, 0x23d60, 0x225d8, 0x220c0, 0x22f70, 0x239f8, 0x22208, 0x21d98, 0x22bc8, 0x226d0, 0x234f8, 0x23050, 0x23e78, 0x239e0, 0x221d0, 0x22d30, 0x22858, 0x23658, 0x231a8, 0x23f78, 0x23ae0, 0x22328, 0x21e68, 0x22d20, 0x22838, 0x21fd0, 0x22e58, 0x22988, 0x23798, 0x232e0, 0x21b50, 0x23c50, 0x22478, 0x21fa8, 0x22ed8, 0x238c0, 0x23420, 0x21c58, 0x23d58, 0x225d0, 0x220b8, 0x22f68, 0x22ac0, 0x238a8, 0x23400, 0x22800, 0x22ce8, 0x21f30, 0x22408, 0x23ba8, 0x21aa0, 0x23288, 0x23718, 0x22910, 0x22de0, 0x23e38, 0x23100, 0x235b0, 0x22780, 0x22c70, 0x21e38, 0x222b0, 0x23a80, 0x23f18, 0x22fe0, 0x225a8, 0x23d28, 0x21c28, 0x233f0, 0x23898, 0x22a80, 0x22f30, 0x22158, 0x22670, 0x23dd0, 0x232c8, 0x23778, 0x22950, 0x22e30, 0x21fa0, 0x22468, 0x23c10, 0x21b10, 0x23398, 0x23840, 0x23f58, 0x23170, 0x23628, 0x227f8, 0x22ce0, 0x21f28, 0x22400, 0x23ba0, 0x21a98, 0x23280, 0x226b0, 0x22b90, 0x21d50, 0x221c0, 0x239b0, 0x23e30, 0x230f8, 0x235a8, 0x22778, 0x22c68, 0x22b20, 0x22fc0, 0x22050, 0x22530, 0x23cc8, 0x21bd8, 0x23368, 0x23818, 0x22a08, 0x22ec0, 0x21b98, 0x23240, 0x236e8, 0x228e0, 0x22dc8, 0x21ee8, 0x223c0, 0x23b70, 0x21a60, 0x23258, 0x231f8, 0x236c0, 0x228b8, 0x22d80, 0x21e10, 0x222a0, 0x23a50, 0x23ef8, 0x230b8, 0x23560, 0x22260, 0x23a28, 0x23ec0, 0x23000, 0x234a8, 0x22648, 0x22b48, 0x21cb0, 0x22128, 0x23928, 0x227a8, 0x22458, 0x21f90, 0x21b00, 0x23c00, 0x23770, 0x232b8, 0x22e28, 0x22948, 0x22220, 0x23290, 0x22de8, 0x22918, 0x22418, 0x21f48, 0x21ac8, 0x23bc8, 0x23888, 0x233c8, 0x22f20, 0x22570, 0x22078, 0x21bf8, 0x23cf8, 0x23860, 0x233b0, 0x22f00, 0x22a48, 0x226b8, 0x22198, 0x22668, 0x22178, 0x21d08, 0x23df0, 0x23970, 0x23618, 0x23160, 0x22cd0, 0x227d8, 0x22300, 0x23f28, 0x23a90, 0x235d0, 0x23120, 0x22c98, 0x227a0, 0x22450, 0x21f88, 0x21af8, 0x23bf8, 0x21b40, 0x23c40, 0x23688, 0x231e0, 0x22d60, 0x22888, 0x22368, 0x21ea0, 0x23fb8, 0x23b20, 0x22ab0, 0x224c0, 0x22000, 0x21b70, 0x23c78, 0x237c0, 0x23318, 0x22e80, 0x229d0, 0x22580, 0x221f0, 0x21d80, 0x23e68, 0x239d0, 0x23450, 0x22fa0, 0x22af8, 0x22608, 0x220f0, 0x21c80, 0x23198, 0x22d10, 0x22820, 0x22230, 0x21dc8, 0x23e90, 0x23a08, 0x23528, 0x23078, 0x22c00, 0x229a8, 0x22488, 0x21fc0, 0x21b38, 0x23c38, 0x23680, 0x231d8, 0x22d58, 0x22880, 0x22360, 0x228d8, 0x22db0, 0x23228, 0x236e0, 0x23b18, 0x23f90, 0x21e90, 0x22340, 0x22840, 0x22cf8, 0x23ee8, 0x21df8, 0x22280, 0x22710, 0x22be0, 0x23068, 0x23510, 0x239e8, 0x23e50, 0x21d60, 0x23488, 0x23918, 0x23d88, 0x21c70, 0x220e0, 0x225f0, 0x22ad0, 0x22f50, 0x23408, 0x238a0, 0x22518, 0x229c0, 0x22e70, 0x23308, 0x237b0, 0x23c58, 0x21b20, 0x21fb0, 0x22470, 0x22978, 0x21a38, 0x21ed8, 0x223a8, 0x228d0, 0x22da8, 0x23220, 0x236d8, 0x23b10, 0x23f88, 0x21e88, 0x22740, 0x22c28, 0x230a8, 0x23548, 0x23a48, 0x23ee0, 0x21df0, 0x22278, 0x22708, 0x22bd8, 0x22620, 0x22b38, 0x22fe8, 0x23480, 0x23910, 0x23d80, 0x21c68, 0x220d8, 0x225e8, 0x22ac8, 0x23340, 0x237f8, 0x23cc0, 0x21bc0, 0x22038, 0x22510, 0x229b8, 0x22e68, 0x23300, 0x237a8, 0x232c0, 0x23768, 0x23ad8, 0x23f68, 0x21e60, 0x22320, 0x227f0, 0x22cd8, 0x23168, 0x23620, 0x222f0, 0x226c0, 0x22ba8, 0x23048, 0x234f0, 0x23998, 0x23e20, 0x21d30, 0x221a8, 0x225c0, 0x22868, 0x23668, 0x231c0, 0x21a50, 0x23b58, 0x223b0, 0x21ed0, 0x22d90, 0x228a0, 0x236a0, 0x224a8, 0x21fe8, 0x22eb0, 0x229f8, 0x23808, 0x23350, 0x21bb0, 0x23c98, 0x224e0, 0x22010, 0x23438, 0x21cc8, 0x23db0, 0x22630, 0x22118, 0x22fd0, 0x22b08, 0x238e8, 0x23460, 0x21c10, 0x226f0, 0x23518, 0x23060, 0x23e88, 0x23a00, 0x22218, 0x21da8, 0x22c38, 0x22738, 0x23550, 0x23f98, 0x23af8, 0x22348, 0x21e80, 0x22d40, 0x22860, 0x23660, 0x231b8, 0x21a48, 0x23b50, 0x232f8, 0x21b60, 0x23c68, 0x224a0, 0x21fe0, 0x22ea8, 0x229f0, 0x23800, 0x23348, 0x21ba8, 0x225e0, 0x220d0, 0x22f88, 0x22ae0, 0x238d0, 0x23430, 0x21cc0, 0x23da8, 0x22628, 0x22110, 0x222b8, 0x21e30, 0x22b78, 0x22688, 0x234c8, 0x23028, 0x23dd8, 0x23960, 0x22160, 0x21cf0, 0x23278, 0x23f30, 0x23aa0, 0x222c8, 0x21e48, 0x22c88, 0x22790, 0x235c0, 0x23110, 0x23ea0, 0x21c00, 0x23d00, 0x22568, 0x22068, 0x22df8, 0x22920, 0x23728, 0x23298, 0x21aa8, 0x23bb0, 0x226a8, 0x22b88, 0x21d48, 0x221b8, 0x239a8, 0x23e28, 0x230f0, 0x235a0, 0x22770, 0x22c60, 0x22b18, 0x22fb8, 0x22048, 0x22528, 0x23cd8, 0x21bd0, 0x23360, 0x23810, 0x22a00, 0x22eb8, 0x21b90, 0x23238, 0x236f8, 0x228f0, 0x22dc0, 0x21ee0, 0x223b8, 0x23b68, 0x21a58, 0x23250, 0x23208, 0x236b8, 0x228b0, 0x22d78, 0x21e08, 0x22298, 0x23a60, 0x23ef0, 0x230b0, 0x23558, 0x22258, 0x23a20, 0x23eb8, 0x22ff8, 0x234a0, 0x22640, 0x22b40, 0x21ca8, 0x22120, 0x23920, 0x21cb8, 0x23470, 0x238f8, 0x22b10, 0x22fb0, 0x22040, 0x22520, 0x23cd0, 0x21bc8, 0x23358, 0x22a10, 0x22ea0, 0x22020, 0x224f8, 0x23ca0, 0x21b88, 0x23230, 0x236f0, 0x228e8, 0x22db8, 0x223c8, 0x23b60, 0x21a40, 0x23200, 0x236b0, 0x228a8, 0x22d70, 0x21e00, 0x22290, 0x23a58, 0x230c0, 0x23568, 0x22748, 0x22c30, 0x21dd8, 0x22250, 0x23a18, 0x23eb0, 0x22ff0, 0x23498, 0x220b0, 0x225b8, 0x23d48, 0x21c48, 0x233e0, 0x23890, 0x22a78, 0x22f28, 0x22098, 0x22588, 0x22660, 0x22170, 0x21d00, 0x23de8, 0x23968, 0x23610, 0x23158, 0x22cc8, 0x227d0, 0x22308, 0x23f20, 0x23a88, 0x235c8, 0x23118, 0x22c90, 0x22798, 0x22460, 0x21f80, 0x21b08, 0x23bf0, 0x21b30, 0x23c30, 0x23678, 0x231d0, 0x22d50, 0x22898, 0x22358, 0x21e98, 0x23fb0, 0x23b28, 0x22aa8, 0x224b8, 0x21ff8, 0x21b80, 0x23c90, 0x237b8, 0x23310, 0x22e78, 0x229c8, 0x22578, 0x221e8, 0x21d78, 0x23e60, 0x239c8, 0x23448, 0x22f98, 0x22af0, 0x22600, 0x220e8, 0x21c78, 0x23190, 0x22d08, 0x22828, 0x22238, 0x21dc0, 0x23e98, 0x23a10, 0x23520, 0x23070, 0x22bf8, 0x229a0, 0x22490, 0x21fb8, 0x21b28, 0x23c28, 0x23670, 0x231c8, 0x22d48, 0x22890, 0x22350, 0x21c88, 0x23d70, 0x238b8, 0x23410, 0x22f58, 0x22aa0, 0x224b0, 0x21ff0, 0x21b78, 0x23c88, 0x23080, 0x22be8, 0x226f8, 0x221e0, 0x21d70, 0x23e58, 0x239c0, 0x23440, 0x22f90, 0x22ae8, 0x22378, 0x21ea8, 0x23fa0, 0x23b00, 0x23640, 0x23188, 0x22d00, 0x22830, 0x22240, 0x21db8, 0x21e20, 0x23f08, 0x23a78, 0x23590, 0x230e8, 0x22c80, 0x22760, 0x222c0, 0x21e40, 0x23ea8, 0x23930, 0x23af0, 0x23008, 0x22b50, 0x22658, 0x22140, 0x21cd8, 0x23dc0, 0x23940, 0x234c0, 0x22a28, 0x22538, 0x226e8, 0x21be0, 0x23ce0, 0x23830, 0x23388, 0x22ef8, 0x22a30, 0x22558, 0x21f08, 0x21a90, 0x23b90, 0x23d68, 0x23248, 0x22dd0, 0x22908, 0x223f8, 0x21f20, 0x21ab0, 0x23578, 0x230d0, 0x22c58, 0x22750, 0x22998, 0x21e18, 0x23f00, 0x23a70, 0x23588, 0x230e0, 0x22090, 0x21db0, 0x23ce8, 0x23828, 0x23370, 0x22ed0, 0x22a20, 0x22550, 0x22190, 0x21d10, 0x22f80, 0x228f8, 0x223e0, 0x21f00, 0x21a78, 0x23b88, 0x23710, 0x233c0, 0x22f08, 0x22a68, 0x23f50, 0x23ab8, 0x235f0, 0x23140, 0x22c48, 0x22758, 0x22438, 0x21f68, 0x21ae0, 0x23be0, 0x226a0, 0x22188, 0x21d28, 0x23db8, 0x23938, 0x235f8, 0x23138, 0x22cb0, 0x227c0, 0x222e0, 0x22f18, 0x22a70, 0x225a0, 0x22210, 0x21d90, 0x23e08, 0x23978, 0x234e0, 0x23030, 0x22b60, 0x224f0, 0x222d8, 0x23ab0, 0x23f40, 0x23150, 0x23608, 0x227e8, 0x22ca8, 0x21e58, 0x22318, 0x23740, 0x22940, 0x22e10, 0x21f78, 0x22448, 0x23c08, 0x21ad8, 0x232d0, 0x23788, 0x22810, 0x22a58, 0x22248, 0x22088, 0x23870, 0x23d20, 0x22f10, 0x233d0, 0x22598, 0x22a60, 0x21c40, 0x23980, 0x23df8, 0x236a8, 0x234d0, 0x22698, 0x22b80, 0x21d20, 0x221a0, 0x23990, 0x23e10, 0x235e8, 0x227c8, 0x22ca0, 0x224e8, 0x222d0, 0x23aa8, 0x23f38, 0x23148, 0x23600, 0x227e0, 0x21f60, 0x22430, 0x23be8, 0x21ad0, 0x238f0, 0x23738, 0x22938, 0x22e08, 0x21f70, 0x22440, 0x22e50, 0x23490, 0x22398, 0x22870, 0x23fc0, 0x21ec8, 0x23698, 0x23b48, 0x22ee0, 0x23378, 0x22288, 0x237d8, 0x23c70, 0x22e88, 0x23338, 0x224d0, 0x229e8, 0x21cd0, 0x22138, 0x23948, 0x226e0, 0x22bc0, 0x21d68, 0x221d8, 0x238d8, 0x23da0, 0x230d8, 0x23580, 0x22768, 0x22c50, 0x23f70, 0x23178, 0x23638, 0x22720, 0x22c20, 0x21f18, 0x223e8, 0x23b98, 0x21a80, 0x23260};

// int w = 0,

int step[1200];
int flag = 0;
static int indexs = 0;

// main: 0x55d5eade17a9 main: 0x1b7a9

int
maze_init() {
  printf("UP112_GOT_MAZE_CHALLENGE\n");
  // printf("_main = %p", __stored_ptr)
  addr = maze_get_ptr();
  printf("SOLVER: _main = <%p>\n", addr);
	// printf("SOLVER: _main = <%p>\n", maze_get_ptr());

  // maze_t* mz = maze_load("./maze.txt");
  for(int i = 0; i < 1200; i++)
    step[i] = 0;
  // solve(m);
	return 0;
}

void solve(maze_t *m){
  // printf("before: %d %d\n", m->cx, m->cy);
  is_visited[m->cy][m->cx] = 1;
  if(m->cx == m-> ex && m->cy == m->ey){
    flag = 1;
    // return;
  }
  if(!flag && m->blk[m->cy+1][m->cx] == 0 && !is_visited[m->cy+1][m->cx]){
    step[indexs] = 0;
    m->cy = m->cy+1;
    indexs++;
    solve(m);
  }

  if(!flag && m->blk[m->cy-1][m->cx] == 0 && !is_visited[m->cy-1][m->cx]){
    step[indexs] = 1;
    m->cy = m->cy-1;
    indexs++;
    solve(m);
  }

  if(!flag && m->blk[m->cy][m->cx+1] == 0 && !is_visited[m->cy][m->cx+1]){
    step[indexs] = 2;
    m->cx = m->cx+1;
    indexs++;
    solve(m);
  }
  if(!flag && m->blk[m->cy][m->cx-1] == 0 && !is_visited[m->cy][m->cx-1]){
    step[indexs] = 3;
    m->cx = m->cx-1;
    indexs++;
    solve(m);
  }


  if(flag)
    return;

  indexs--;
  switch(step[indexs]){
    case 0:
      m->cy = m->cy-1;
      break;
    case 1:
      m->cy = m->cy+1;
      break;
    case 2:
      m->cx = m->cx-1;
      break;
    case 3:
      m->cx = m->cx+1;
      break;
  }
  // printf("back: %d %d\n", m->cx, m->cy);
  return;
}

void move_1(maze_t *m){
  solve(m);
  // m->cx = m->ex;
  // m->cy = m->ey;
  m->cx = m->sx;
  m->cy = m->sy;
  void *handle = dlopen("libmaze.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "Error: %s\n", dlerror());
    return;
  }

  // funcPtr up = dlsym(handle, "move_up");
  // funcPtr down = dlsym(handle, "move_down");
  // funcPtr left = dlsym(handle, "move_left");
  // funcPtr right = dlsym(handle, "move_right");
  void* up = dlsym(handle, "move_up");
  void* down = dlsym(handle, "move_down");
  void* left = dlsym(handle, "move_left");
  void* right = dlsym(handle, "move_right");
  if (!up || !down || !left || !right) {
    fprintf(stderr, "Error: %s\n", dlerror());
    dlclose(handle);
    return;
  }
  // printf("1\n");
  int pagesize = sysconf(_SC_PAGE_SIZE);
  // printf("2\n");
  // printf("%d\n", pagesize);
  // printf("%p\n", (void*)((unsigned long int)((char*)addr-MAIN_OFFSET+0x21a38) & ~(pagesize-1)));
  void *start = (void*)((unsigned long int)((char*)addr-MAIN_OFFSET+0x21a38) & ~(pagesize-1));
  int size = (8*1200 + pagesize-1) & ~(pagesize-1);
  // printf("3\n");
  if (mprotect(start, size, PROT_READ | PROT_WRITE) == -1) {
    perror("mprotect");
    dlclose(handle);
    return;
  }
  // printf("%d\n", indexs);
  for(int i = 0; i < indexs; i++){
    // printf("run: %d %d\n", m->cy, m->cx);
    // if(step[i] == -1)
    //   break;
    // printf("%p\n", (void*)((char*)addr-MAIN_OFFSET+func_offset[i+1]));
    switch(step[i]){
      case 0:
        // *((char*)addr-MAIN_OFFSET+func_offset[i+1]) = &down;
        memcpy((void*)((char*)addr-MAIN_OFFSET+func_offset[i+1]), &down, 8);
        // (*down)(m);
        // move_down(m);
        break;
      case 1:
        // *((char*)addr-MAIN_OFFSET+func_offset[i+1]) = &up;
        memcpy((void*)((char*)addr-MAIN_OFFSET+func_offset[i+1]), &up, 8);
        // (*up)(m);
        // move_up(m);
        break;
      case 2:
        // *((char*)addr-MAIN_OFFSET+func_offset[i+1]) = &right;
        memcpy((void*)((char*)addr-MAIN_OFFSET+func_offset[i+1]), &right, 8);
        // (*right)(m);
        // move_right(m);
        break;
      case 3:
        // *((char*)addr-MAIN_OFFSET+func_offset[i+1]) = &left;
        memcpy((void*)((char*)addr-MAIN_OFFSET+func_offset[i+1]), &left, 8);
        // (*left)(m);
        // move_left(m);
        break;
    }

  }
  dlclose(handle);
  // printf("before: %d %d\n", m->cy, m->cx);
}