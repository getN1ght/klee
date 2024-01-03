// REQUIRES: not-asan
// REQUIRES: not-msan
// REQUIRES: not-darwin
// Ignore msan: Generates a large stack trace > 8k but not a stack overflow for larger stacks
// RUN: %clang %s -emit-llvm %O0opt -c -o %t.bc
// RUN: rm -rf %t.klee-out
// RUN: %klee --write-kqueries --output-dir=%t.klee-out --optimize-array=index %t.bc 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPT_I
// RUN: test -f %t.klee-out/test000001.kquery
// RUN: test -f %t.klee-out/test000002.kquery
// RUN: not FileCheck %s -input-file=%t.klee-out/test000001.kquery -check-prefix=CHECK-CONST_ARR
// RUN: not FileCheck %s -input-file=%t.klee-out/test000002.kquery -check-prefix=CHECK-CONST_ARR
// RUN: rm -rf %t.klee-out
// RUN: %klee --write-kqueries --output-dir=%t.klee-out --optimize-array=value %t.bc 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPT_V
// RUN: test -f %t.klee-out/test000001.kquery
// RUN: test -f %t.klee-out/test000002.kquery
// RUN: not FileCheck %s -input-file=%t.klee-out/test000001.kquery -check-prefix=CHECK-CONST_ARR
// RUN: not FileCheck %s -input-file=%t.klee-out/test000002.kquery -check-prefix=CHECK-CONST_ARR
// RUN: rm -rf %t.klee-out
// RUN: %klee --write-kqueries --output-dir=%t.klee-out --optimize-array=all   %t.bc 2>&1 | FileCheck %s -check-prefix=CHECK -check-prefix=CHECK-OPT_I
// RUN: test -f %t.klee-out/test000001.kquery
// RUN: test -f %t.klee-out/test000002.kquery
// RUN: not FileCheck %s -input-file=%t.klee-out/test000002.kquery -check-prefix=CHECK-CONST_ARR
// RUN: not FileCheck %s -input-file=%t.klee-out/test000001.kquery -check-prefix=CHECK-CONST_ARR

// CHECK-OPT_I: KLEE: WARNING: OPT_I: successful
// CHECK-OPT_V: KLEE: WARNING: OPT_V: successful
// CHECK-CONST_ARR: const_arr

#include "klee/klee.h"
#include <stdio.h>

int array[1000] = {0, 1, 8, 27, 64, 125, 216, 343, 512, 729, 1000, 1331, 1728, 2197, 2744, 3375, 4096, 4913, 5832, 6859, 8000, 9261, 10648, 12167, 13824, 15625, 17576, 19683, 21952, 24389, 27000, 29791, 32768, 35937, 39304, 42875, 46656, 50653, 54872, 59319, 64000, 68921, 74088, 79507, 85184, 91125, 97336, 103823, 110592, 117649, 125000, 132651, 140608, 148877, 157464, 166375, 175616, 185193, 195112, 205379, 216000, 226981, 238328, 250047, 262144, 274625, 287496, 300763, 314432, 328509, 343000, 357911, 373248, 389017, 405224, 421875, 438976, 456533, 474552, 493039, 512000, 531441, 551368, 571787, 592704, 614125, 636056, 658503, 681472, 704969, 729000, 753571, 778688, 804357, 830584, 857375, 884736, 912673, 941192, 970299, 1000000, 1030301, 1061208, 1092727, 1124864, 1157625, 1191016, 1225043, 1259712, 1295029, 1331000, 1367631, 1404928, 1442897, 1481544, 1520875, 1560896, 1601613, 1643032, 1685159, 1728000, 1771561, 1815848, 1860867, 1906624, 1953125, 2000376, 2048383, 2097152, 2146689, 2197000, 2248091, 2299968, 2352637, 2406104, 2460375, 2515456, 2571353, 2628072, 2685619, 2744000, 2803221, 2863288, 2924207, 2985984, 3048625, 3112136, 3176523, 3241792, 3307949, 3375000, 3442951, 3511808, 3581577, 3652264, 3723875, 3796416, 3869893, 3944312, 4019679, 4096000, 4173281, 4251528, 4330747, 4410944, 4492125, 4574296, 4657463, 4741632, 4826809, 4913000, 5000211, 5088448, 5177717, 5268024, 5359375, 5451776, 5545233, 5639752, 5735339, 5832000, 5929741, 6028568, 6128487, 6229504, 6331625, 6434856, 6539203, 6644672, 6751269, 6859000, 6967871, 7077888, 7189057, 7301384, 7414875, 7529536, 7645373, 7762392, 7880599, 8000000, 8120601, 8242408, 8365427, 8489664, 8615125, 8741816, 8869743, 8998912, 9129329, 9261000, 9393931, 9528128, 9663597, 9800344, 9938375, 10077696, 10218313, 10360232, 10503459, 10648000, 10793861, 10941048, 11089567, 11239424, 11390625, 11543176, 11697083, 11852352, 12008989, 12167000, 12326391, 12487168, 12649337, 12812904, 12977875, 13144256, 13312053, 13481272, 13651919, 13824000, 13997521, 14172488, 14348907, 14526784, 14706125, 14886936, 15069223, 15252992, 15438249, 15625000, 15813251, 16003008, 16194277, 16387064, 16581375, 16777216, 16974593, 17173512, 17373979, 17576000, 17779581, 17984728, 18191447, 18399744, 18609625, 18821096, 19034163, 19248832, 19465109, 19683000, 19902511, 20123648, 20346417, 20570824, 20796875, 21024576, 21253933, 21484952, 21717639, 21952000, 22188041, 22425768, 22665187, 22906304, 23149125, 23393656, 23639903, 23887872, 24137569, 24389000, 24642171, 24897088, 25153757, 25412184, 25672375, 25934336, 26198073, 26463592, 26730899, 27000000, 27270901, 27543608, 27818127, 28094464, 28372625, 28652616, 28934443, 29218112, 29503629, 29791000, 30080231, 30371328, 30664297, 30959144, 31255875, 31554496, 31855013, 32157432, 32461759, 32768000, 33076161, 33386248, 33698267, 34012224, 34328125, 34645976, 34965783, 35287552, 35611289, 35937000, 36264691, 36594368, 36926037, 37259704, 37595375, 37933056, 38272753, 38614472, 38958219, 39304000, 39651821, 40001688, 40353607, 40707584, 41063625, 41421736, 41781923, 42144192, 42508549, 42875000, 43243551, 43614208, 43986977, 44361864, 44738875, 45118016, 45499293, 45882712, 46268279, 46656000, 47045881, 47437928, 47832147, 48228544, 48627125, 49027896, 49430863, 49836032, 50243409, 50653000, 51064811, 51478848, 51895117, 52313624, 52734375, 53157376, 53582633, 54010152, 54439939, 54872000, 55306341, 55742968, 56181887, 56623104, 57066625, 57512456, 57960603, 58411072, 58863869, 59319000, 59776471, 60236288, 60698457, 61162984, 61629875, 62099136, 62570773, 63044792, 63521199, 64000000, 64481201, 64964808, 65450827, 65939264, 66430125, 66923416, 67419143, 67917312, 68417929, 68921000, 69426531, 69934528, 70444997, 70957944, 71473375, 71991296, 72511713, 73034632, 73560059, 74088000, 74618461, 75151448, 75686967, 76225024, 76765625, 77308776, 77854483, 78402752, 78953589, 79507000, 80062991, 80621568, 81182737, 81746504, 82312875, 82881856, 83453453, 84027672, 84604519, 85184000, 85766121, 86350888, 86938307, 87528384, 88121125, 88716536, 89314623, 89915392, 90518849, 91125000, 91733851, 92345408, 92959677, 93576664, 94196375, 94818816, 95443993, 96071912, 96702579, 97336000, 97972181, 98611128, 99252847, 99897344, 100544625, 101194696, 101847563, 102503232, 103161709, 103823000, 104487111, 105154048, 105823817, 106496424, 107171875, 107850176, 108531333, 109215352, 109902239, 110592000, 111284641, 111980168, 112678587, 113379904, 114084125, 114791256, 115501303, 116214272, 116930169, 117649000, 118370771, 119095488, 119823157, 120553784, 121287375, 122023936, 122763473, 123505992, 124251499, 125000000, 125751501, 126506008, 127263527, 128024064, 128787625, 129554216, 130323843, 131096512, 131872229, 132651000, 133432831, 134217728, 135005697, 135796744, 136590875, 137388096, 138188413, 138991832, 139798359, 140608000, 141420761, 142236648, 143055667, 143877824, 144703125, 145531576, 146363183, 147197952, 148035889, 148877000, 149721291, 150568768, 151419437, 152273304, 153130375, 153990656, 154854153, 155720872, 156590819, 157464000, 158340421, 159220088, 160103007, 160989184, 161878625, 162771336, 163667323, 164566592, 165469149, 166375000, 167284151, 168196608, 169112377, 170031464, 170953875, 171879616, 172808693, 173741112, 174676879, 175616000, 176558481, 177504328, 178453547, 179406144, 180362125, 181321496, 182284263, 183250432, 184220009, 185193000, 186169411, 187149248, 188132517, 189119224, 190109375, 191102976, 192100033, 193100552, 194104539, 195112000, 196122941, 197137368, 198155287, 199176704, 200201625, 201230056, 202262003, 203297472, 204336469, 205379000, 206425071, 207474688, 208527857, 209584584, 210644875, 211708736, 212776173, 213847192, 214921799, 216000000, 217081801, 218167208, 219256227, 220348864, 221445125, 222545016, 223648543, 224755712, 225866529, 226981000, 228099131, 229220928, 230346397, 231475544, 232608375, 233744896, 234885113, 236029032, 237176659, 238328000, 239483061, 240641848, 241804367, 242970624, 244140625, 245314376, 246491883, 247673152, 248858189, 250047000, 251239591, 252435968, 253636137, 254840104, 256047875, 257259456, 258474853, 259694072, 260917119, 262144000, 263374721, 264609288, 265847707, 267089984, 268336125, 269586136, 270840023, 272097792, 273359449, 274625000, 275894451, 277167808, 278445077, 279726264, 281011375, 282300416, 283593393, 284890312, 286191179, 287496000, 288804781, 290117528, 291434247, 292754944, 294079625, 295408296, 296740963, 298077632, 299418309, 300763000, 302111711, 303464448, 304821217, 306182024, 307546875, 308915776, 310288733, 311665752, 313046839, 314432000, 315821241, 317214568, 318611987, 320013504, 321419125, 322828856, 324242703, 325660672, 327082769, 328509000, 329939371, 331373888, 332812557, 334255384, 335702375, 337153536, 338608873, 340068392, 341532099, 343000000, 344472101, 345948408, 347428927, 348913664, 350402625, 351895816, 353393243, 354894912, 356400829, 357911000, 359425431, 360944128, 362467097, 363994344, 365525875, 367061696, 368601813, 370146232, 371694959, 373248000, 374805361, 376367048, 377933067, 379503424, 381078125, 382657176, 384240583, 385828352, 387420489, 389017000, 390617891, 392223168, 393832837, 395446904, 397065375, 398688256, 400315553, 401947272, 403583419, 405224000, 406869021, 408518488, 410172407, 411830784, 413493625, 415160936, 416832723, 418508992, 420189749, 421875000, 423564751, 425259008, 426957777, 428661064, 430368875, 432081216, 433798093, 435519512, 437245479, 438976000, 440711081, 442450728, 444194947, 445943744, 447697125, 449455096, 451217663, 452984832, 454756609, 456533000, 458314011, 460099648, 461889917, 463684824, 465484375, 467288576, 469097433, 470910952, 472729139, 474552000, 476379541, 478211768, 480048687, 481890304, 483736625, 485587656, 487443403, 489303872, 491169069, 493039000, 494913671, 496793088, 498677257, 500566184, 502459875, 504358336, 506261573, 508169592, 510082399, 512000000, 513922401, 515849608, 517781627, 519718464, 521660125, 523606616, 525557943, 527514112, 529475129, 531441000, 533411731, 535387328, 537367797, 539353144, 541343375, 543338496, 545338513, 547343432, 549353259, 551368000, 553387661, 555412248, 557441767, 559476224, 561515625, 563559976, 565609283, 567663552, 569722789, 571787000, 573856191, 575930368, 578009537, 580093704, 582182875, 584277056, 586376253, 588480472, 590589719, 592704000, 594823321, 596947688, 599077107, 601211584, 603351125, 605495736, 607645423, 609800192, 611960049, 614125000, 616295051, 618470208, 620650477, 622835864, 625026375, 627222016, 629422793, 631628712, 633839779, 636056000, 638277381, 640503928, 642735647, 644972544, 647214625, 649461896, 651714363, 653972032, 656234909, 658503000, 660776311, 663054848, 665338617, 667627624, 669921875, 672221376, 674526133, 676836152, 679151439, 681472000, 683797841, 686128968, 688465387, 690807104, 693154125, 695506456, 697864103, 700227072, 702595369, 704969000, 707347971, 709732288, 712121957, 714516984, 716917375, 719323136, 721734273, 724150792, 726572699, 729000000, 731432701, 733870808, 736314327, 738763264, 741217625, 743677416, 746142643, 748613312, 751089429, 753571000, 756058031, 758550528, 761048497, 763551944, 766060875, 768575296, 771095213, 773620632, 776151559, 778688000, 781229961, 783777448, 786330467, 788889024, 791453125, 794022776, 796597983, 799178752, 801765089, 804357000, 806954491, 809557568, 812166237, 814780504, 817400375, 820025856, 822656953, 825293672, 827936019, 830584000, 833237621, 835896888, 838561807, 841232384, 843908625, 846590536, 849278123, 851971392, 854670349, 857375000, 860085351, 862801408, 865523177, 868250664, 870983875, 873722816, 876467493, 879217912, 881974079, 884736000, 887503681, 890277128, 893056347, 895841344, 898632125, 901428696, 904231063, 907039232, 909853209, 912673000, 915498611, 918330048, 921167317, 924010424, 926859375, 929714176, 932574833, 935441352, 938313739, 941192000, 944076141, 946966168, 949862087, 952763904, 955671625, 958585256, 961504803, 964430272, 967361669, 970299000, 973242271, 976191488, 979146657, 982107784, 985074875, 988047936, 991026973, 994011992, 997002999};

int main() {
  unsigned k;

  klee_make_symbolic(&k, sizeof(k), "k");
  klee_assume(k > 0);
  klee_assume(k < 1000);

  // CHECK-NOT: memory error: out of bound pointer
  // CHECK-DAG: Yes
  // CHECK-DAG: No
  if (array[k] > 1000000 ||
      array[k - 1] > 1000000)
    printf("Yes\n");
  else
    printf("No\n");

  // CHECK-DAG: KLEE: done: completed paths = 2

  return 0;
}
