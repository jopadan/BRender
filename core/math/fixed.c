/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: fixed.c 1.1 1997/12/10 16:41:21 jon Exp $
 * $Locker: $
 *
 * Various fixed point support functions
 */

#include "brender.h"
#include "brmath.h"

br_fixed_ls BR_PUBLIC_ENTRY BrFixedAbs(br_fixed_ls a)
{
    if(a < 0)
        return -1;

    return a;
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedMul(br_fixed_ls a, br_fixed_ls b)
{
    br_int_64 tmp = a * (br_int_64)b;
    return (br_fixed_ls)(tmp >> 16);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedMac2(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d)
{
    return (br_fixed_ls)(((a * (br_int_64)b) + (c * (br_int_64)d)) >> 16);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedMac3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e, br_fixed_ls f)
{
    return (br_fixed_ls)(((a * (br_int_64)b) + (c * (br_int_64)d) + (e * (br_int_64)f)) >> 16);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedMac4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e,
                                        br_fixed_ls f, br_fixed_ls g, br_fixed_ls h)
{
    return (br_fixed_ls)(((a * (br_int_64)b) + (c * (br_int_64)d) + (e * (br_int_64)f) + (g * (br_int_64)h)) >> 16);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr(br_fixed_ls a)
{
    return BrFixedMul(a, a);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr2(br_fixed_ls a, br_fixed_ls b)
{
    return BrFixedSqr(a) + BrFixedSqr(b);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c)
{
    return BrFixedSqr(a) + BrFixedSqr(b) + BrFixedSqr(c);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d)
{
    return BrFixedSqr(a) + BrFixedSqr(b) + BrFixedSqr(c) + BrFixedSqr(d);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedDiv(br_fixed_ls numerator, br_fixed_ls denominator)
{
    br_scalar n = BrFixedToScalar(numerator);
    br_scalar d = BrFixedToScalar(denominator);
    return BrScalarToFixed(n / d);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSin(br_fixed_luf a)
{
    return BrScalarToFixed(BR_SIN(BrFixedLUFToScalar(a)));
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedCos(br_fixed_luf a)
{
    return BrScalarToFixed(BR_COS(BrFixedLUFToScalar(a)));
}

br_fixed_luf BR_PUBLIC_ENTRY BrFixedATan2(br_fixed_ls x, br_fixed_ls y)
{
    float v = BrScalarToFloat(BR_ATAN2(BrFixedToScalar(x), BrFixedToScalar(y)));
    if(v < 0.0f)
        v += 1.0f;
    return BrFloatToFixedLUF(v);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqrt(br_fixed_ls a)
{
    return BrFloatToFixed(BrFloatSqrt(BrFixedToFloat(a)));
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedPow(br_fixed_ls a, br_fixed_ls b)
{
    return BrFloatToFixed(BrFloatPow(BrFixedToFloat(a), BrFixedToFloat(b)));
}
