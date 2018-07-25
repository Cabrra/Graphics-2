#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 6.3.9600.16384
//
//
// Buffer Definitions: 
//
// cbuffer OBJECT
// {
//
//   row_major float4x4 worldMatrix;    // Offset:    0 Size:    64
//
// }
//
// cbuffer SCENE
// {
//
//   row_major float4x4 viewMatrix;     // Offset:    0 Size:    64
//   row_major float4x4 projectionMatrix;// Offset:   64 Size:    64
//
// }
//
// cbuffer TIME
// {
//
//   float4 elapsedTime;                // Offset:    0 Size:    16
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim Slot Elements
// ------------------------------ ---------- ------- ----------- ---- --------
// OBJECT                            cbuffer      NA          NA    0        1
// SCENE                             cbuffer      NA          NA    1        1
// TIME                              cbuffer      NA          NA    2        1
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// UV                       0   xyz         0     NONE   float   xyz 
// NORMAL                   0   xyz         1     NONE   float   xyz 
// POSITION                 0   xyz         2     NONE   float   x z 
// TANGENT                  0   xyz         3     NONE   float       
// SV_INSTANCEID            0   x           4   INSTID    uint       
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// UV                       0   xyz         0     NONE   float   xyz 
// NORMAL                   0   xyz         1     NONE   float   xyz 
// SV_POSITION              0   xyzw        2      POS   float   xyzw
// POSITION                 0   xyz         3     NONE   float   xyz 
//
vs_4_0
dcl_constantbuffer cb0[4], immediateIndexed
dcl_constantbuffer cb1[8], immediateIndexed
dcl_constantbuffer cb2[1], immediateIndexed
dcl_input v0.xyz
dcl_input v1.xyz
dcl_input v2.xz
dcl_output o0.xyz
dcl_output o1.xyz
dcl_output_siv o2.xyzw, position
dcl_output o3.xyz
dcl_temps 2
mov o0.xyz, v0.xyzx
mul r0.xyz, v1.yyyy, cb0[1].xyzx
mad r0.xyz, v1.xxxx, cb0[0].xyzx, r0.xyzx
mad o1.xyz, v1.zzzz, cb0[2].xyzx, r0.xyzx
mul r0.x, cb2[0].x, l(2.500000)
mad r0.x, v2.z, l(-0.500000), -r0.x
sincos r0.x, null, r0.x
add r0.x, r0.x, l(-1.500000)
mul r0.xyzw, r0.xxxx, cb0[1].xyzw
add r1.x, v2.z, cb2[0].x
sincos null, r1.y, r1.x
sincos r1.x, null, -r1.x
add r1.x, r1.x, r1.y
add r1.y, cb2[0].x, cb2[0].x
sincos null, r1.y, r1.y
add r1.x, -r1.y, r1.x
add r1.x, -r1.x, v2.x
mad r0.xyzw, r1.xxxx, cb0[0].xyzw, r0.xyzw
add r1.x, r1.x, cb2[0].x
sincos null, r1.x, r1.x
mad r1.x, -r1.x, l(0.200000), v2.z
mad r0.xyzw, r1.xxxx, cb0[2].xyzw, r0.xyzw
add r0.xyzw, r0.xyzw, cb0[3].xyzw
mul r1.xyzw, r0.yyyy, cb1[1].xyzw
mad r1.xyzw, r0.xxxx, cb1[0].xyzw, r1.xyzw
mad r1.xyzw, r0.zzzz, cb1[2].xyzw, r1.xyzw
mad r0.xyzw, r0.wwww, cb1[3].xyzw, r1.xyzw
mul r1.xyzw, r0.yyyy, cb1[5].xyzw
mad r1.xyzw, r0.xxxx, cb1[4].xyzw, r1.xyzw
mad r1.xyzw, r0.zzzz, cb1[6].xyzw, r1.xyzw
mad o2.xyzw, r0.wwww, cb1[7].xyzw, r1.xyzw
mov o3.xz, v2.xxzx
mov o3.y, l(-0.500000)
ret 
// Approximately 34 instruction slots used
#endif

const BYTE waterVShader[] =
{
     68,  88,  66,  67, 196, 152, 
     24, 163,   5,  93, 235,  26, 
     35,  37,  44,  14, 108, 124, 
      0, 106,   1,   0,   0,   0, 
    148,   8,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
    252,   1,   0,   0, 176,   2, 
      0,   0,  64,   3,   0,   0, 
     24,   8,   0,   0,  82,  68, 
     69,  70, 192,   1,   0,   0, 
      3,   0,   0,   0, 144,   0, 
      0,   0,   3,   0,   0,   0, 
     28,   0,   0,   0,   0,   4, 
    254, 255,   0,   1,   0,   0, 
    140,   1,   0,   0, 124,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
    131,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0, 137,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  79,  66, 
     74,  69,  67,  84,   0,  83, 
     67,  69,  78,  69,   0,  84, 
     73,  77,  69,   0, 171, 171, 
    124,   0,   0,   0,   1,   0, 
      0,   0, 216,   0,   0,   0, 
     64,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    131,   0,   0,   0,   2,   0, 
      0,   0,  12,   1,   0,   0, 
    128,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    137,   0,   0,   0,   1,   0, 
      0,   0,  88,   1,   0,   0, 
     16,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
    240,   0,   0,   0,   0,   0, 
      0,   0,  64,   0,   0,   0, 
      2,   0,   0,   0, 252,   0, 
      0,   0,   0,   0,   0,   0, 
    119, 111, 114, 108, 100,  77, 
     97, 116, 114, 105, 120,   0, 
      2,   0,   3,   0,   4,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  60,   1, 
      0,   0,   0,   0,   0,   0, 
     64,   0,   0,   0,   2,   0, 
      0,   0, 252,   0,   0,   0, 
      0,   0,   0,   0,  71,   1, 
      0,   0,  64,   0,   0,   0, 
     64,   0,   0,   0,   2,   0, 
      0,   0, 252,   0,   0,   0, 
      0,   0,   0,   0, 118, 105, 
    101, 119,  77,  97, 116, 114, 
    105, 120,   0, 112, 114, 111, 
    106, 101,  99, 116, 105, 111, 
    110,  77,  97, 116, 114, 105, 
    120,   0, 112,   1,   0,   0, 
      0,   0,   0,   0,  16,   0, 
      0,   0,   2,   0,   0,   0, 
    124,   1,   0,   0,   0,   0, 
      0,   0, 101, 108,  97, 112, 
    115, 101, 100,  84, 105, 109, 
    101,   0,   1,   0,   3,   0, 
      1,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     77, 105,  99, 114, 111, 115, 
    111, 102, 116,  32,  40,  82, 
     41,  32,  72,  76,  83,  76, 
     32,  83, 104,  97, 100, 101, 
    114,  32,  67, 111, 109, 112, 
    105, 108, 101, 114,  32,  54, 
     46,  51,  46,  57,  54,  48, 
     48,  46,  49,  54,  51,  56, 
     52,   0, 171, 171,  73,  83, 
     71,  78, 172,   0,   0,   0, 
      5,   0,   0,   0,   8,   0, 
      0,   0, 128,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   7,   7, 
      0,   0, 131,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,   7,   7, 
      0,   0, 138,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      2,   0,   0,   0,   7,   5, 
      0,   0, 147,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      3,   0,   0,   0,   7,   0, 
      0,   0, 155,   0,   0,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,   1,   0,   0,   0, 
      4,   0,   0,   0,   1,   0, 
      0,   0,  85,  86,   0,  78, 
     79,  82,  77,  65,  76,   0, 
     80,  79,  83,  73,  84,  73, 
     79,  78,   0,  84,  65,  78, 
     71,  69,  78,  84,   0,  83, 
     86,  95,  73,  78,  83,  84, 
     65,  78,  67,  69,  73,  68, 
      0, 171, 171, 171,  79,  83, 
     71,  78, 136,   0,   0,   0, 
      4,   0,   0,   0,   8,   0, 
      0,   0, 104,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   7,   8, 
      0,   0, 107,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,   7,   8, 
      0,   0, 114,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
      2,   0,   0,   0,  15,   0, 
      0,   0, 126,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      3,   0,   0,   0,   7,   8, 
      0,   0,  85,  86,   0,  78, 
     79,  82,  77,  65,  76,   0, 
     83,  86,  95,  80,  79,  83, 
     73,  84,  73,  79,  78,   0, 
     80,  79,  83,  73,  84,  73, 
     79,  78,   0, 171,  83,  72, 
     68,  82, 208,   4,   0,   0, 
     64,   0,   1,   0,  52,   1, 
      0,   0,  89,   0,   0,   4, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   4,   0,   0,   0, 
     89,   0,   0,   4,  70, 142, 
     32,   0,   1,   0,   0,   0, 
      8,   0,   0,   0,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      2,   0,   0,   0,   1,   0, 
      0,   0,  95,   0,   0,   3, 
    114,  16,  16,   0,   0,   0, 
      0,   0,  95,   0,   0,   3, 
    114,  16,  16,   0,   1,   0, 
      0,   0,  95,   0,   0,   3, 
     82,  16,  16,   0,   2,   0, 
      0,   0, 101,   0,   0,   3, 
    114,  32,  16,   0,   0,   0, 
      0,   0, 101,   0,   0,   3, 
    114,  32,  16,   0,   1,   0, 
      0,   0, 103,   0,   0,   4, 
    242,  32,  16,   0,   2,   0, 
      0,   0,   1,   0,   0,   0, 
    101,   0,   0,   3, 114,  32, 
     16,   0,   3,   0,   0,   0, 
    104,   0,   0,   2,   2,   0, 
      0,   0,  54,   0,   0,   5, 
    114,  32,  16,   0,   0,   0, 
      0,   0,  70,  18,  16,   0, 
      0,   0,   0,   0,  56,   0, 
      0,   8, 114,   0,  16,   0, 
      0,   0,   0,   0,  86,  21, 
     16,   0,   1,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     50,   0,   0,  10, 114,   0, 
     16,   0,   0,   0,   0,   0, 
      6,  16,  16,   0,   1,   0, 
      0,   0,  70, 130,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  70,   2,  16,   0, 
      0,   0,   0,   0,  50,   0, 
      0,  10, 114,  32,  16,   0, 
      1,   0,   0,   0, 166,  26, 
     16,   0,   1,   0,   0,   0, 
     70, 130,  32,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
     70,   2,  16,   0,   0,   0, 
      0,   0,  56,   0,   0,   8, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10, 128,  32,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,  32,  64,  50,   0, 
      0,  10,  18,   0,  16,   0, 
      0,   0,   0,   0,  42,  16, 
     16,   0,   2,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0, 191,  10,   0,  16, 128, 
     65,   0,   0,   0,   0,   0, 
      0,   0,  77,   0,   0,   6, 
     18,   0,  16,   0,   0,   0, 
      0,   0,   0, 208,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,   0,   0,   0,   7, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 192, 191, 
     56,   0,   0,   8, 242,   0, 
     16,   0,   0,   0,   0,   0, 
      6,   0,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   8, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  42,  16,  16,   0, 
      2,   0,   0,   0,  10, 128, 
     32,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,  77,   0, 
      0,   6,   0, 208,   0,   0, 
     34,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  77,   0, 
      0,   7,  18,   0,  16,   0, 
      1,   0,   0,   0,   0, 208, 
      0,   0,  10,   0,  16, 128, 
     65,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   7, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  26,   0, 
     16,   0,   1,   0,   0,   0, 
      0,   0,   0,   9,  34,   0, 
     16,   0,   1,   0,   0,   0, 
     10, 128,  32,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
     10, 128,  32,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
     77,   0,   0,   6,   0, 208, 
      0,   0,  34,   0,  16,   0, 
      1,   0,   0,   0,  26,   0, 
     16,   0,   1,   0,   0,   0, 
      0,   0,   0,   8,  18,   0, 
     16,   0,   1,   0,   0,   0, 
     26,   0,  16, 128,  65,   0, 
      0,   0,   1,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,   0,   0,   0,   8, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16, 128, 
     65,   0,   0,   0,   1,   0, 
      0,   0,  10,  16,  16,   0, 
      2,   0,   0,   0,  50,   0, 
      0,  10, 242,   0,  16,   0, 
      0,   0,   0,   0,   6,   0, 
     16,   0,   1,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,   0,   0,   0,   8, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  10, 128, 
     32,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,  77,   0, 
      0,   6,   0, 208,   0,   0, 
     18,   0,  16,   0,   1,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10,  18,   0,  16,   0, 
      1,   0,   0,   0,  10,   0, 
     16, 128,  65,   0,   0,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0, 205, 204,  76,  62, 
     42,  16,  16,   0,   2,   0, 
      0,   0,  50,   0,   0,  10, 
    242,   0,  16,   0,   0,   0, 
      0,   0,   6,   0,  16,   0, 
      1,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
      0,   0,   0,   8, 242,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  56,   0,   0,   8, 
    242,   0,  16,   0,   1,   0, 
      0,   0,  86,   5,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10, 242,   0,  16,   0, 
      1,   0,   0,   0,   6,   0, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
    242,   0,  16,   0,   1,   0, 
      0,   0, 166,  10,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   1,   0,   0,   0, 
      2,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     50,   0,   0,  10, 242,   0, 
     16,   0,   0,   0,   0,   0, 
    246,  15,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  56,   0, 
      0,   8, 242,   0,  16,   0, 
      1,   0,   0,   0,  86,   5, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   1,   0, 
      0,   0,   5,   0,   0,   0, 
     50,   0,   0,  10, 242,   0, 
     16,   0,   1,   0,   0,   0, 
      6,   0,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      1,   0,   0,   0,   4,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  50,   0, 
      0,  10, 242,   0,  16,   0, 
      1,   0,   0,   0, 166,  10, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   1,   0, 
      0,   0,   6,   0,   0,   0, 
     70,  14,  16,   0,   1,   0, 
      0,   0,  50,   0,   0,  10, 
    242,  32,  16,   0,   2,   0, 
      0,   0, 246,  15,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   1,   0,   0,   0, 
      7,   0,   0,   0,  70,  14, 
     16,   0,   1,   0,   0,   0, 
     54,   0,   0,   5,  82,  32, 
     16,   0,   3,   0,   0,   0, 
      6,  18,  16,   0,   2,   0, 
      0,   0,  54,   0,   0,   5, 
     34,  32,  16,   0,   3,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0,   0, 191,  62,   0, 
      0,   1,  83,  84,  65,  84, 
    116,   0,   0,   0,  34,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   7,   0, 
      0,   0,  30,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0
};