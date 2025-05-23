/* { dg-do assemble { target aarch64_asm_sve2p1_ok } } */
/* { dg-do compile { target { ! aarch64_asm_sve2p1_ok } } } */
/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

#pragma GCC target "+sve2p1"
#ifdef STREAMING_COMPATIBLE
#pragma GCC target "+sme2"
#endif

/*
** pext_lane_p2_pn0_0:
**	mov	p([0-9]+)\.b, p0\.b
**	pext	{p2\.h, p3\.h}, pn\1\[0\]
**	ret
*/
TEST_EXTRACT_PN (pext_lane_p2_pn0_0, svboolx2_t,
		 p2 = svpext_lane_c16_x2 (pn0, 0),
		 p2 = svpext_lane_c16_x2 (pn0, 0))

/*
** pext_lane_p5_pn7_1:
**	mov	p([0-9]+)\.b, p7\.b
**	pext	{[^}]+}, pn\1\[1\]
**	mov	[^\n]+
**	mov	[^\n]+
**	ret
*/
TEST_EXTRACT_PN (pext_lane_p5_pn7_1, svboolx2_t,
		 p5 = svpext_lane_c16_x2 (pn7, 1),
		 p5 = svpext_lane_c16_x2 (pn7, 1))

/*
** pext_lane_p9_pn8_0:
**	pext	{[^}]+}, pn8\[0\]
**	mov	[^\n]+
**	mov	[^\n]+
**	ret
*/
TEST_EXTRACT_PN (pext_lane_p9_pn8_0, svboolx2_t,
		 p9 = svpext_lane_c16_x2 (pn8, 0),
		 p9 = svpext_lane_c16_x2 (pn8, 0))

/*
** pext_lane_p12_pn11_1:
**	pext	{p12\.h, p13\.h}, pn11\[1\]
**	ret
*/
TEST_EXTRACT_PN (pext_lane_p12_pn11_1, svboolx2_t,
		 p12 = svpext_lane_c16_x2 (pn11, 1),
		 p12 = svpext_lane_c16_x2 (pn11, 1))

/*
** pext_lane_p2_pn15_0:
**	pext	{p2\.h, p3\.h}, pn15\[0\]
**	ret
*/
TEST_EXTRACT_PN (pext_lane_p2_pn15_0, svboolx2_t,
		 p2 = svpext_lane_c16_x2 (pn15, 0),
		 p2 = svpext_lane_c16_x2 (pn15, 0))
