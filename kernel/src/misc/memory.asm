bits 64

section .text

global memsetPage

memsetPage:
    movq xmm0, rsi
    jmp .L2
.L3:
    movaps [rdi], xmm0
    movaps [rdi+16], xmm0
    movaps [rdi+32], xmm0
    movaps [rdi+48], xmm0
    movaps [rdi+64], xmm0
    movaps [rdi+80], xmm0
    movaps [rdi+96], xmm0
    movaps [rdi+112], xmm0
    movaps [rdi+128], xmm0
    movaps [rdi+144], xmm0
    movaps [rdi+160], xmm0
    movaps [rdi+176], xmm0
    movaps [rdi+192], xmm0
    movaps [rdi+208], xmm0
    movaps [rdi+224], xmm0
    movaps [rdi+240], xmm0
    movaps [rdi+256], xmm0
    movaps [rdi+272], xmm0
    movaps [rdi+288], xmm0
    movaps [rdi+304], xmm0
    movaps [rdi+320], xmm0
    movaps [rdi+336], xmm0
    movaps [rdi+352], xmm0
    movaps [rdi+368], xmm0
    movaps [rdi+384], xmm0
    movaps [rdi+400], xmm0
    movaps [rdi+416], xmm0
    movaps [rdi+432], xmm0
    movaps [rdi+448], xmm0
    movaps [rdi+464], xmm0
    movaps [rdi+480], xmm0
    movaps [rdi+496], xmm0
    movaps [rdi+512], xmm0
    movaps [rdi+528], xmm0
    movaps [rdi+544], xmm0
    movaps [rdi+560], xmm0
    movaps [rdi+576], xmm0
    movaps [rdi+592], xmm0
    movaps [rdi+608], xmm0
    movaps [rdi+624], xmm0
    movaps [rdi+640], xmm0
    movaps [rdi+656], xmm0
    movaps [rdi+672], xmm0
    movaps [rdi+688], xmm0
    movaps [rdi+704], xmm0
    movaps [rdi+720], xmm0
    movaps [rdi+736], xmm0
    movaps [rdi+752], xmm0
    movaps [rdi+768], xmm0
    movaps [rdi+784], xmm0
    movaps [rdi+800], xmm0
    movaps [rdi+816], xmm0
    movaps [rdi+832], xmm0
    movaps [rdi+848], xmm0
    movaps [rdi+864], xmm0
    movaps [rdi+880], xmm0
    movaps [rdi+896], xmm0
    movaps [rdi+912], xmm0
    movaps [rdi+928], xmm0
    movaps [rdi+944], xmm0
    movaps [rdi+960], xmm0
    movaps [rdi+976], xmm0
    movaps [rdi+992], xmm0
    movaps [rdi+1008], xmm0
    movaps [rdi+1024], xmm0
    movaps [rdi+1040], xmm0
    movaps [rdi+1056], xmm0
    movaps [rdi+1072], xmm0
    movaps [rdi+1088], xmm0
    movaps [rdi+1104], xmm0
    movaps [rdi+1120], xmm0
    movaps [rdi+1136], xmm0
    movaps [rdi+1152], xmm0
    movaps [rdi+1168], xmm0
    movaps [rdi+1184], xmm0
    movaps [rdi+1200], xmm0
    movaps [rdi+1216], xmm0
    movaps [rdi+1232], xmm0
    movaps [rdi+1248], xmm0
    movaps [rdi+1264], xmm0
    movaps [rdi+1280], xmm0
    movaps [rdi+1296], xmm0
    movaps [rdi+1312], xmm0
    movaps [rdi+1328], xmm0
    movaps [rdi+1344], xmm0
    movaps [rdi+1360], xmm0
    movaps [rdi+1376], xmm0
    movaps [rdi+1392], xmm0
    movaps [rdi+1408], xmm0
    movaps [rdi+1424], xmm0
    movaps [rdi+1440], xmm0
    movaps [rdi+1456], xmm0
    movaps [rdi+1472], xmm0
    movaps [rdi+1488], xmm0
    movaps [rdi+1504], xmm0
    movaps [rdi+1520], xmm0
    movaps [rdi+1536], xmm0
    movaps [rdi+1552], xmm0
    movaps [rdi+1568], xmm0
    movaps [rdi+1584], xmm0
    movaps [rdi+1600], xmm0
    movaps [rdi+1616], xmm0
    movaps [rdi+1632], xmm0
    movaps [rdi+1648], xmm0
    movaps [rdi+1664], xmm0
    movaps [rdi+1680], xmm0
    movaps [rdi+1696], xmm0
    movaps [rdi+1712], xmm0
    movaps [rdi+1728], xmm0
    movaps [rdi+1744], xmm0
    movaps [rdi+1760], xmm0
    movaps [rdi+1776], xmm0
    movaps [rdi+1792], xmm0
    movaps [rdi+1808], xmm0
    movaps [rdi+1824], xmm0
    movaps [rdi+1840], xmm0
    movaps [rdi+1856], xmm0
    movaps [rdi+1872], xmm0
    movaps [rdi+1888], xmm0
    movaps [rdi+1904], xmm0
    movaps [rdi+1920], xmm0
    movaps [rdi+1936], xmm0
    movaps [rdi+1952], xmm0
    movaps [rdi+1968], xmm0
    movaps [rdi+1984], xmm0
    movaps [rdi+2000], xmm0
    movaps [rdi+2016], xmm0
    movaps [rdi+2032], xmm0
    movaps [rdi+2048], xmm0
    movaps [rdi+2064], xmm0
    movaps [rdi+2080], xmm0
    movaps [rdi+2096], xmm0
    movaps [rdi+2112], xmm0
    movaps [rdi+2128], xmm0
    movaps [rdi+2144], xmm0
    movaps [rdi+2160], xmm0
    movaps [rdi+2176], xmm0
    movaps [rdi+2192], xmm0
    movaps [rdi+2208], xmm0
    movaps [rdi+2224], xmm0
    movaps [rdi+2240], xmm0
    movaps [rdi+2256], xmm0
    movaps [rdi+2272], xmm0
    movaps [rdi+2288], xmm0
    movaps [rdi+2304], xmm0
    movaps [rdi+2320], xmm0
    movaps [rdi+2336], xmm0
    movaps [rdi+2352], xmm0
    movaps [rdi+2368], xmm0
    movaps [rdi+2384], xmm0
    movaps [rdi+2400], xmm0
    movaps [rdi+2416], xmm0
    movaps [rdi+2432], xmm0
    movaps [rdi+2448], xmm0
    movaps [rdi+2464], xmm0
    movaps [rdi+2480], xmm0
    movaps [rdi+2496], xmm0
    movaps [rdi+2512], xmm0
    movaps [rdi+2528], xmm0
    movaps [rdi+2544], xmm0
    movaps [rdi+2560], xmm0
    movaps [rdi+2576], xmm0
    movaps [rdi+2592], xmm0
    movaps [rdi+2608], xmm0
    movaps [rdi+2624], xmm0
    movaps [rdi+2640], xmm0
    movaps [rdi+2656], xmm0
    movaps [rdi+2672], xmm0
    movaps [rdi+2688], xmm0
    movaps [rdi+2704], xmm0
    movaps [rdi+2720], xmm0
    movaps [rdi+2736], xmm0
    movaps [rdi+2752], xmm0
    movaps [rdi+2768], xmm0
    movaps [rdi+2784], xmm0
    movaps [rdi+2800], xmm0
    movaps [rdi+2816], xmm0
    movaps [rdi+2832], xmm0
    movaps [rdi+2848], xmm0
    movaps [rdi+2864], xmm0
    movaps [rdi+2880], xmm0
    movaps [rdi+2896], xmm0
    movaps [rdi+2912], xmm0
    movaps [rdi+2928], xmm0
    movaps [rdi+2944], xmm0
    movaps [rdi+2960], xmm0
    movaps [rdi+2976], xmm0
    movaps [rdi+2992], xmm0
    movaps [rdi+3008], xmm0
    movaps [rdi+3024], xmm0
    movaps [rdi+3040], xmm0
    movaps [rdi+3056], xmm0
    movaps [rdi+3072], xmm0
    movaps [rdi+3088], xmm0
    movaps [rdi+3104], xmm0
    movaps [rdi+3120], xmm0
    movaps [rdi+3136], xmm0
    movaps [rdi+3152], xmm0
    movaps [rdi+3168], xmm0
    movaps [rdi+3184], xmm0
    movaps [rdi+3200], xmm0
    movaps [rdi+3216], xmm0
    movaps [rdi+3232], xmm0
    movaps [rdi+3248], xmm0
    movaps [rdi+3264], xmm0
    movaps [rdi+3280], xmm0
    movaps [rdi+3296], xmm0
    movaps [rdi+3312], xmm0
    movaps [rdi+3328], xmm0
    movaps [rdi+3344], xmm0
    movaps [rdi+3360], xmm0
    movaps [rdi+3376], xmm0
    movaps [rdi+3392], xmm0
    movaps [rdi+3408], xmm0
    movaps [rdi+3424], xmm0
    movaps [rdi+3440], xmm0
    movaps [rdi+3456], xmm0
    movaps [rdi+3472], xmm0
    movaps [rdi+3488], xmm0
    movaps [rdi+3504], xmm0
    movaps [rdi+3520], xmm0
    movaps [rdi+3536], xmm0
    movaps [rdi+3552], xmm0
    movaps [rdi+3568], xmm0
    movaps [rdi+3584], xmm0
    movaps [rdi+3600], xmm0
    movaps [rdi+3616], xmm0
    movaps [rdi+3632], xmm0
    movaps [rdi+3648], xmm0
    movaps [rdi+3664], xmm0
    movaps [rdi+3680], xmm0
    movaps [rdi+3696], xmm0
    movaps [rdi+3712], xmm0
    movaps [rdi+3728], xmm0
    movaps [rdi+3744], xmm0
    movaps [rdi+3760], xmm0
    movaps [rdi+3776], xmm0
    movaps [rdi+3792], xmm0
    movaps [rdi+3808], xmm0
    movaps [rdi+3824], xmm0
    movaps [rdi+3840], xmm0
    movaps [rdi+3856], xmm0
    movaps [rdi+3872], xmm0
    movaps [rdi+3888], xmm0
    movaps [rdi+3904], xmm0
    movaps [rdi+3920], xmm0
    movaps [rdi+3936], xmm0
    movaps [rdi+3952], xmm0
    movaps [rdi+3968], xmm0
    movaps [rdi+3984], xmm0
    movaps [rdi+4000], xmm0
    movaps [rdi+4016], xmm0
    movaps [rdi+4032], xmm0
    movaps [rdi+4048], xmm0
    movaps [rdi+4064], xmm0
    movaps [rdi+4080], xmm0

    dec rdx
    add rdi, 4096
.L2:
    test rdx, rdx
    jne .L3
    ret