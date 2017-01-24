
/**
 * @file /magma/providers/checkers/allocations.h
 *
 * @brief Functions used to scan, analyze, check, and validate data.
 *
 *
 * Allocation Data
 * http://www.iana.org/assignments/ipv4-address-space/ipv4-address-space.xml
 * http://www.iana.org/assignments/ipv4-address-space/ipv4-address-space.txt
 *
 * Network Popularity
 * http://www.spamcop.net/w3m?action=map
 * http://www.spamcop.net/w3m?action=map;format=text
 *
 * Last Updated
 * 2010/11/09
 */

#ifndef MAGMA_PROVIDERS_CHECKERS_ALLOCATIONS_H
#define MAGMA_PROVIDERS_CHECKERS_ALLOCATIONS_H

/*****************************************************************************************************************************


*****************************************************************************************************************************/

const struct {
	uint32_t status, owner, weight;
	chr_t *comment;
} ip_v4_allocations_t[] = {
	{
		// 0.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 1.0.0.0/24
		ALLOCATED,
		REGISTRY,
		0,
		"APNIC"
	},
	{
		// 2.0.0.0/24
		ALLOCATED,
		REGISTRY,
		0,
		"RIPE NCC"
	},
	{
		// 3.0.0.0/24
		ALLOCATED,
		DIRECT,
		434,
		"GENERAL ELECTRIC COMPANY"
	},
	{
		// 4.0.0.0/24
		ALLOCATED,
		DIRECT,
		9113,
		"LEVEL 3 COMMUNICATIONS"
	},
	{
		// 5.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 6.0.0.0/24
		ALLOCATED,
		DIRECT,
		178,
		"ARMY INFORMATION SYSTEMS CENTER"
	},
	{
		// 7.0.0.0/24
		ALLOCATED,
		REGISTRY,
		0,
		"ARIN"
	},
	{
		// 8.0.0.0/24
		ALLOCATED,
		DIRECT,
		7178,
		"LEVEL 3 COMMUNICATIONS"
	},
	{
		// 9.0.0.0/24
		ALLOCATED,
		DIRECT,
		766,
		"IBM"
	},
	{
		// 10.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 11.0.0.0/24
		ALLOCATED,
		DIRECT,
		436,
		"DOD INTEL INFORMATION SYSTEMS"
	},
	{
		// 12.0.0.0/24
		ALLOCATED,
		DIRECT,
		40243,
		"AT&T BELL LABORATORIES"
	},
	{
		// 13.0.0.0/24
		ALLOCATED,
		DIRECT,
		292,
		"XEROX CORPORATION"
	},
	{
		// 14.0.0.0/24
		ALLOCATED,
		REGISTRY,
		0,
		"APNIC"
	},
	{
		// 15.0.0.0/24
		ALLOCATED,
		DIRECT,
		497,
		"HEWLETT-PACKARD COMPANY"
	},
	{
		// 16.0.0.0/24
		ALLOCATED,
		DIRECT,
		589,
		"DIGITAL EQUIPMENT CORPORATION"
	},
	{
		// 17.0.0.0/24
		ALLOCATED,
		DIRECT,
		645,
		"APPLE COMPUTER"
	},
	{
		// 18.0.0.0/24
		ALLOCATED,
		DIRECT,
		685,
		"MIT"
	},
	{
		// 19.0.0.0/24
		ALLOCATED,
		DIRECT,
		280,
		"FORD MOTOR COMPANY"
	},
	{
		// 20.0.0.0/24
		ALLOCATED,
		DIRECT,
		452,
		"COMPUTER SCIENCES CORPORATION"
	},
	{
		// 21.0.0.0/24
		ALLOCATED,
		DIRECT,
		182,
		"DDN-RVN"
	},
	{
		// 22.0.0.0/24
		ALLOCATED,
		DIRECT,
		225,
		"DEFENSE INFORMATION SYSTEMS AGENCY"
	},
	{
		// 23.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 24.0.0.0/24
		ALLOCATED,
		REGISTRY,
		134794,
		"ARIN"
	},
	{
		// 25.0.0.0/24
		ALLOCATED,
		DIRECT,
		258,
		"UK MINISTRY OF DEFENCE"
	},
	{
		// 26.0.0.0/24
		ALLOCATED,
		DIRECT,
		221,
		"DEFENSE INFORMATION SYSTEMS AGENCY"
	},
	{
		// 27.0.0.0/24
		ALLOCATED,
		REGISTRY,
		19540,
		"APNIC"
	},
	{
		// 28.0.0.0/24
		ALLOCATED,
		DIRECT,
		321,
		"DSI-NORTH"
	},
	{
		// 29.0.0.0/24
		ALLOCATED,
		DIRECT,
		219,
		"DEFENSE INFORMATION SYSTEMS AGENCY"
	},
	{
		// 30.0.0.0/24
		ALLOCATED,
		DIRECT,
		271,
		"DEFENSE INFORMATION SYSTEMS AGENCY"
	},
	{
		// 31.0.0.0/24
		ALLOCATED,
		REGISTRY,
		101,
		"RIPE NCC"
	},
	{
		// 32.0.0.0/24
		ALLOCATED,
		DIRECT,
		1725,
		"AT&T GLOBAL NETWORK SERVICES"
	},
	{
		// 33.0.0.0/24
		ALLOCATED,
		DIRECT,
		194,
		"DLA SYSTEMS AUTOMATION CENTER"
	},
	{
		// 34.0.0.0/24
		ALLOCATED,
		DIRECT,
		299,
		"HALLIBURTON COMPANY"
	},
	{
		// 35.0.0.0/24
		ALLOCATED,
		DIRECT,
		1144,
		"MERIT COMPUTER NETWORK"
	},
	{
		// 36.0.0.0/24
		ALLOCATED,
		REGISTRY,
		139,
		"APNIC"
	},
	{
		// 37.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 38.0.0.0/24
		ALLOCATED,
		DIRECT,
		10908,
		"PSINET"
	},
	{
		// 39.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 40.0.0.0/24
		ALLOCATED,
		DIRECT,
		321,
		"ELI LILY & COMPANY"
	},
	{
		// 41.0.0.0/24
		ALLOCATED,
		REGISTRY,
		427763,
		"AFRINIC"
	},
	{
		// 42.0.0.0/24
		ALLOCATED,
		REGISTRY,
		109,
		"APNIC"
	},
	{
		// 43.0.0.0/24
		ALLOCATED,
		REGISTRY,
		638,
		"APNIC"
	},
	{
		// 44.0.0.0/24
		ALLOCATED,
		DIRECT,
		284,
		"AMATEUR RADIO DIGITAL COMMUNICATIONS"
	},
	{
		// 45.0.0.0/24
		ALLOCATED,
		DIRECT,
		208,
		"INTEROP SHOW NETWORK"
	},
	{
		// 46.0.0.0/24
		ALLOCATED,
		REGISTRY,
		37725,
		"RIPE NCC"
	},
	{
		// 47.0.0.0/24
		ALLOCATED,
		DIRECT,
		221,
		"BELL-NORTHERN RESEARCH"
	},
	{
		// 48.0.0.0/24
		ALLOCATED,
		DIRECT,
		367,
		"PRUDENTIAL SECURITIES"
	},
	{
		// 49.0.0.0/24
		ALLOCATED,
		REGISTRY,
		208,
		"APNIC"
	},
	{
		// 50.0.0.0/24
		ALLOCATED,
		REGISTRY,
		1122,
		"ARIN"
	},
	{
		// 51.0.0.0/24
		ALLOCATED,
		DIRECT,
		252,
		"UK GOVERNMENT DEPARTMENT FOR WORK AND PENSIONS"
	},
	{
		// 52.0.0.0/24
		ALLOCATED,
		DIRECT,
		273,
		"EI DUPONT DE NEMOURS AND CO"
	},
	{
		// 53.0.0.0/24
		ALLOCATED,
		DIRECT,
		261,
		"CAP DEBIS CCS"
	},
	{
		// 54.0.0.0/24
		ALLOCATED,
		DIRECT,
		320,
		"MERCK AND CO"
	},
	{
		// 55.0.0.0/24
		ALLOCATED,
		DIRECT,
		350,
		"DOD NETWORK INFORMATION CENTER"
	},
	{
		// 56.0.0.0/24
		ALLOCATED,
		DIRECT,
		366,
		"US POSTAL SERVICE"
	},
	{
		// 57.0.0.0/24
		ALLOCATED,
		DIRECT,
		708,
		"SITA"
	},
	{
		// 58.0.0.0/24
		ALLOCATED,
		REGISTRY,
		212498,
		"APNIC"
	},
	{
		// 59.0.0.0/24
		ALLOCATED,
		REGISTRY,
		340981,
		"APNIC"
	},
	{
		// 60.0.0.0/24
		ALLOCATED,
		REGISTRY,
		127272,
		"APNIC"
	},
	{
		// 61.0.0.0/24
		ALLOCATED,
		REGISTRY,
		164146,
		"APNIC"
	},
	{
		// 62.0.0.0/24
		ALLOCATED,
		REGISTRY,
		162116,
		"RIPE NCC"
	},
	{
		// 63.0.0.0/24
		ALLOCATED,
		REGISTRY,
		50412,
		"ARIN"
	},
	{
		// 64.0.0.0/24
		ALLOCATED,
		REGISTRY,
		90532,
		"ARIN"
	},
	{
		// 65.0.0.0/24
		ALLOCATED,
		REGISTRY,
		65347,
		"ARIN"
	},
	{
		// 66.0.0.0/24
		ALLOCATED,
		REGISTRY,
		133837,
		"ARIN"
	},
	{
		// 67.0.0.0/24
		ALLOCATED,
		REGISTRY,
		110877,
		"ARIN"
	},
	{
		// 68.0.0.0/24
		ALLOCATED,
		REGISTRY,
		103413,
		"ARIN"
	},
	{
		// 69.0.0.0/24
		ALLOCATED,
		REGISTRY,
		123901,
		"ARIN"
	},
	{
		// 70.0.0.0/24
		ALLOCATED,
		REGISTRY,
		109800,
		"ARIN"
	},
	{
		// 71.0.0.0/24
		ALLOCATED,
		REGISTRY,
		163828,
		"ARIN"
	},
	{
		// 72.0.0.0/24
		ALLOCATED,
		REGISTRY,
		110224,
		"ARIN"
	},
	{
		// 73.0.0.0/24
		ALLOCATED,
		REGISTRY,
		91,
		"ARIN"
	},
	{
		// 74.0.0.0/24
		ALLOCATED,
		REGISTRY,
		79245,
		"ARIN"
	},
	{
		// 75.0.0.0/24
		ALLOCATED,
		REGISTRY,
		77784,
		"ARIN"
	},
	{
		// 76.0.0.0/24
		ALLOCATED,
		REGISTRY,
		61095,
		"ARIN"
	},
	{
		// 77.0.0.0/24
		ALLOCATED,
		REGISTRY,
		291140,
		"RIPE NCC"
	},
	{
		// 78.0.0.0/24
		ALLOCATED,
		REGISTRY,
		163642,
		"RIPE NCC"
	},
	{
		// 79.0.0.0/24
		ALLOCATED,
		REGISTRY,
		207160,
		"RIPE NCC"
	},
	{
		// 80.0.0.0/24
		ALLOCATED,
		REGISTRY,
		204358,
		"RIPE NCC"
	},
	{
		// 81.0.0.0/24
		ALLOCATED,
		REGISTRY,
		214351,
		"RIPE NCC"
	},
	{
		// 82.0.0.0/24
		ALLOCATED,
		REGISTRY,
		235586,
		"RIPE NCC"
	},
	{
		// 83.0.0.0/24
		ALLOCATED,
		REGISTRY,
		200177,
		"RIPE NCC"
	},
	{
		// 84.0.0.0/24
		ALLOCATED,
		REGISTRY,
		287060,
		"RIPE NCC"
	},
	{
		// 85.0.0.0/24
		ALLOCATED,
		REGISTRY,
		232369,
		"RIPE NCC"
	},
	{
		// 86.0.0.0/24
		ALLOCATED,
		REGISTRY,
		222364,
		"RIPE NCC"
	},
	{
		// 87.0.0.0/24
		ALLOCATED,
		REGISTRY,
		193718,
		"RIPE NCC"
	},
	{
		// 88.0.0.0/24
		ALLOCATED,
		REGISTRY,
		209331,
		"RIPE NCC"
	},
	{
		// 89.0.0.0/24
		ALLOCATED,
		REGISTRY,
		273166,
		"RIPE NCC"
	},
	{
		// 90.0.0.0/24
		ALLOCATED,
		REGISTRY,
		138596,
		"RIPE NCC"
	},
	{
		// 91.0.0.0/24
		ALLOCATED,
		REGISTRY,
		189388,
		"RIPE NCC"
	},
	{
		// 92.0.0.0/24
		ALLOCATED,
		REGISTRY,
		299631,
		"RIPE NCC"
	},
	{
		// 93.0.0.0/24
		ALLOCATED,
		REGISTRY,
		196553,
		"RIPE NCC"
	},
	{
		// 94.0.0.0/24
		ALLOCATED,
		REGISTRY,
		326009,
		"RIPE NCC"
	},
	{
		// 95.0.0.0/24
		ALLOCATED,
		REGISTRY,
		427549,
		"RIPE NCC"
	},
	{
		// 96.0.0.0/24
		ALLOCATED,
		REGISTRY,
		21913,
		"ARIN"
	},
	{
		// 97.0.0.0/24
		ALLOCATED,
		REGISTRY,
		13721,
		"ARIN"
	},
	{
		// 98.0.0.0/24
		ALLOCATED,
		REGISTRY,
		34764,
		"ARIN"
	},
	{
		// 99.0.0.0/24
		ALLOCATED,
		REGISTRY,
		12832,
		"ARIN"
	},
	{
		// 100.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 101.0.0.0/24
		ALLOCATED,
		REGISTRY,
		83,
		"APNIC"
	},
	{
		// 102.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 103.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 104.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 105.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 106.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 107.0.0.0/24
		ALLOCATED,
		REGISTRY,
		113,
		"ARIN"
	},
	{
		// 108.0.0.0/24
		ALLOCATED,
		REGISTRY,
		1907,
		"ARIN"
	},
	{
		// 109.0.0.0/24
		ALLOCATED,
		REGISTRY,
		235582,
		"RIPE NCC"
	},
	{
		// 110.0.0.0/24
		ALLOCATED,
		REGISTRY,
		92047,
		"APNIC"
	},
	{
		// 111.0.0.0/24
		ALLOCATED,
		REGISTRY,
		28936,
		"APNIC"
	},
	{
		// 112.0.0.0/24
		ALLOCATED,
		REGISTRY,
		49989,
		"APNIC"
	},
	{
		// 113.0.0.0/24
		ALLOCATED,
		REGISTRY,
		255152,
		"APNIC"
	},
	{
		// 114.0.0.0/24
		ALLOCATED,
		REGISTRY,
		74173,
		"APNIC"
	},
	{
		// 115.0.0.0/24
		ALLOCATED,
		REGISTRY,
		161491,
		"APNIC"
	},
	{
		// 116.0.0.0/24
		ALLOCATED,
		REGISTRY,
		103288,
		"APNIC"
	},
	{
		// 117.0.0.0/24
		ALLOCATED,
		REGISTRY,
		378097,
		"APNIC"
	},
	{
		// 118.0.0.0/24
		ALLOCATED,
		REGISTRY,
		91640,
		"APNIC"
	},
	{
		// 119.0.0.0/24
		ALLOCATED,
		REGISTRY,
		187104,
		"APNIC"
	},
	{
		// 120.0.0.0/24
		ALLOCATED,
		REGISTRY,
		66136,
		"APNIC"
	},
	{
		// 121.0.0.0/24
		ALLOCATED,
		REGISTRY,
		116971,
		"APNIC"
	},
	{
		// 122.0.0.0/24
		ALLOCATED,
		REGISTRY,
		298350,
		"APNIC"
	},
	{
		// 123.0.0.0/24
		ALLOCATED,
		REGISTRY,
		234043,
		"APNIC"
	},
	{
		// 124.0.0.0/24
		ALLOCATED,
		REGISTRY,
		163853,
		"APNIC"
	},
	{
		// 125.0.0.0/24
		ALLOCATED,
		REGISTRY,
		201198,
		"APNIC"
	},
	{
		// 126.0.0.0/24
		ALLOCATED,
		REGISTRY,
		308,
		"APNIC"
	},
	{
		// 127.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 128.0.0.0/24
		ALLOCATED,
		REGISTRY,
		19138,
		"ARIN"
	},
	{
		// 129.0.0.0/24
		ALLOCATED,
		REGISTRY,
		10939,
		"ARIN"
	},
	{
		// 130.0.0.0/24
		ALLOCATED,
		REGISTRY,
		13655,
		"ARIN"
	},
	{
		// 131.0.0.0/24
		ALLOCATED,
		REGISTRY,
		5772,
		"ARIN"
	},
	{
		// 132.0.0.0/24
		ALLOCATED,
		REGISTRY,
		4182,
		"ARIN"
	},
	{
		// 133.0.0.0/24
		ALLOCATED,
		REGISTRY,
		3186,
		"APNIC"
	},
	{
		// 134.0.0.0/24
		ALLOCATED,
		REGISTRY,
		5116,
		"ARIN"
	},
	{
		// 135.0.0.0/24
		ALLOCATED,
		REGISTRY,
		1177,
		"ARIN"
	},
	{
		// 136.0.0.0/24
		ALLOCATED,
		REGISTRY,
		1683,
		"ARIN"
	},
	{
		// 137.0.0.0/24
		ALLOCATED,
		REGISTRY,
		6484,
		"ARIN"
	},
	{
		// 138.0.0.0/24
		ALLOCATED,
		REGISTRY,
		7538,
		"ARIN"
	},
	{
		// 139.0.0.0/24
		ALLOCATED,
		REGISTRY,
		4032,
		"ARIN"
	},
	{
		// 140.0.0.0/24
		ALLOCATED,
		REGISTRY,
		9141,
		"ARIN"
	},
	{
		// 141.0.0.0/24
		ALLOCATED,
		REGISTRY,
		14108,
		"RIPE NCC"
	},
	{
		// 142.0.0.0/24
		ALLOCATED,
		REGISTRY,
		11515,
		"ARIN"
	},
	{
		// 143.0.0.0/24
		ALLOCATED,
		REGISTRY,
		4720,
		"ARIN"
	},
	{
		// 144.0.0.0/24
		ALLOCATED,
		REGISTRY,
		7414,
		"ARIN"
	},
	{
		// 145.0.0.0/24
		ALLOCATED,
		REGISTRY,
		4412,
		"RIPE NCC"
	},
	{
		// 146.0.0.0/24
		ALLOCATED,
		REGISTRY,
		3753,
		"ARIN"
	},
	{
		// 147.0.0.0/24
		ALLOCATED,
		REGISTRY,
		7714,
		"ARIN"
	},
	{
		// 148.0.0.0/24
		ALLOCATED,
		REGISTRY,
		8583,
		"ARIN"
	},
	{
		// 149.0.0.0/24
		ALLOCATED,
		REGISTRY,
		3643,
		"ARIN"
	},
	{
		// 150.0.0.0/24
		ALLOCATED,
		REGISTRY,
		6633,
		"APNIC"
	},
	{
		// 151.0.0.0/24
		ALLOCATED,
		REGISTRY,
		136969,
		"RIPE NCC"
	},
	{
		// 152.0.0.0/24
		ALLOCATED,
		REGISTRY,
		3395,
		"ARIN"
	},
	{
		// 153.0.0.0/24
		ALLOCATED,
		REGISTRY,
		1615,
		"APNIC"
	},
	{
		// 154.0.0.0/24
		ALLOCATED,
		REGISTRY,
		612,
		"AFRINIC"
	},
	{
		// 155.0.0.0/24
		ALLOCATED,
		REGISTRY,
		4502,
		"ARIN"
	},
	{
		// 156.0.0.0/24
		ALLOCATED,
		REGISTRY,
		2391,
		"ARIN"
	},
	{
		// 157.0.0.0/24
		ALLOCATED,
		REGISTRY,
		8185,
		"ARIN"
	},
	{
		// 158.0.0.0/24
		ALLOCATED,
		REGISTRY,
		3056,
		"ARIN"
	},
	{
		// 159.0.0.0/24
		ALLOCATED,
		REGISTRY,
		5396,
		"ARIN"
	},
	{
		// 160.0.0.0/24
		ALLOCATED,
		REGISTRY,
		3583,
		"ARIN"
	},
	{
		// 161.0.0.0/24
		ALLOCATED,
		REGISTRY,
		4493,
		"ARIN"
	},
	{
		// 162.0.0.0/24
		ALLOCATED,
		REGISTRY,
		5901,
		"ARIN"
	},
	{
		// 163.0.0.0/24
		ALLOCATED,
		REGISTRY,
		4940,
		"APNIC"
	},
	{
		// 164.0.0.0/24
		ALLOCATED,
		REGISTRY,
		3904,
		"ARIN"
	},
	{
		// 165.0.0.0/24
		ALLOCATED,
		REGISTRY,
		12778,
		"ARIN"
	},
	{
		// 166.0.0.0/24
		ALLOCATED,
		REGISTRY,
		20079,
		"ARIN"
	},
	{
		// 167.0.0.0/24
		ALLOCATED,
		REGISTRY,
		2937,
		"ARIN"
	},
	{
		// 168.0.0.0/24
		ALLOCATED,
		REGISTRY,
		12878,
		"ARIN"
	},
	{
		// 169.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 170.0.0.0/24
		ALLOCATED,
		REGISTRY,
		8840,
		"ARIN"
	},
	{
		// 171.0.0.0/24
		ALLOCATED,
		REGISTRY,
		697,
		"APNIC"
	},
	{
		// 172.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 173.0.0.0/24
		ALLOCATED,
		REGISTRY,
		56003,
		"ARIN"
	},
	{
		// 174.0.0.0/24
		ALLOCATED,
		REGISTRY,
		42547,
		"ARIN"
	},
	{
		// 175.0.0.0/24
		ALLOCATED,
		REGISTRY,
		12484,
		"APNIC"
	},
	{
		// 176.0.0.0/24
		ALLOCATED,
		REGISTRY,
		314,
		"RIPE NCC"
	},
	{
		// 177.0.0.0/24
		ALLOCATED,
		REGISTRY,
		196,
		"LACNIC"
	},
	{
		// 178.0.0.0/24
		ALLOCATED,
		REGISTRY,
		374629,
		"RIPE NCC"
	},
	{
		// 179.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 180.0.0.0/24
		ALLOCATED,
		REGISTRY,
		53379,
		"APNIC"
	},
	{
		// 181.0.0.0/24
		ALLOCATED,
		REGISTRY,
		223,
		"LACNIC"
	},
	{
		// 182.0.0.0/24
		ALLOCATED,
		REGISTRY,
		41815,
		"APNIC"
	},
	{
		// 183.0.0.0/24
		ALLOCATED,
		REGISTRY,
		82046,
		"APNIC"
	},
	{
		// 184.0.0.0/24
		ALLOCATED,
		REGISTRY,
		11086,
		"ARIN"
	},
	{
		// 185.0.0.0/24
		UNALLOCATED,
		EMPTY,
		0,
		NULL
	},
	{
		// 186.0.0.0/24
		ALLOCATED,
		REGISTRY,
		187821,
		"LACNIC"
	},
	{
		// 187.0.0.0/24
		ALLOCATED,
		REGISTRY,
		266272,
		"LACNIC"
	},
	{
		// 188.0.0.0/24
		ALLOCATED,
		REGISTRY,
		280462,
		"RIPE NCC"
	},
	{
		// 189.0.0.0/24
		ALLOCATED,
		REGISTRY,
		383283,
		"LACNIC"
	},
	{
		// 190.0.0.0/24
		ALLOCATED,
		REGISTRY,
		512680,
		"LACNIC"
	},
	{
		// 191.0.0.0/24
		ALLOCATED,
		REGISTRY,
		548,
		"LACNIC"
	},
	{
		// 192.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 193.0.0.0/24
		ALLOCATED,
		REGISTRY,
		69224,
		"RIPE NCC"
	},
	{
		// 194.0.0.0/24
		ALLOCATED,
		REGISTRY,
		58961,
		"RIPE NCC"
	},
	{
		// 195.0.0.0/24
		ALLOCATED,
		REGISTRY,
		100333,
		"RIPE NCC"
	},
	{
		// 196.0.0.0/24
		ALLOCATED,
		REGISTRY,
		88902,
		"AFRINIC"
	},
	{
		// 197.0.0.0/24
		ALLOCATED,
		REGISTRY,
		5403,
		"AFRINIC"
	},
	{
		// 198.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 199.0.0.0/24
		ALLOCATED,
		REGISTRY,
		15080,
		"ARIN"
	},
	{
		// 200.0.0.0/24
		ALLOCATED,
		REGISTRY,
		269919,
		"LACNIC"
	},
	{
		// 201.0.0.0/24
		ALLOCATED,
		REGISTRY,
		409518,
		"LACNIC"
	},
	{
		// 202.0.0.0/24
		ALLOCATED,
		REGISTRY,
		106109,
		"APNIC"
	},
	{
		// 203.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 204.0.0.0/24
		ALLOCATED,
		REGISTRY,
		35015,
		"ARIN"
	},
	{
		// 205.0.0.0/24
		ALLOCATED,
		REGISTRY,
		14164,
		"ARIN"
	},
	{
		// 206.0.0.0/24
		ALLOCATED,
		REGISTRY,
		28730,
		"ARIN"
	},
	{
		// 207.0.0.0/24
		ALLOCATED,
		REGISTRY,
		66000,
		"ARIN"
	},
	{
		// 208.0.0.0/24
		ALLOCATED,
		REGISTRY,
		70335,
		"ARIN"
	},
	{
		// 209.0.0.0/24
		ALLOCATED,
		REGISTRY,
		76247,
		"ARIN"
	},
	{
		// 210.0.0.0/24
		ALLOCATED,
		REGISTRY,
		92815,
		"APNIC"
	},
	{
		// 211.0.0.0/24
		ALLOCATED,
		REGISTRY,
		128852,
		"APNIC"
	},
	{
		// 212.0.0.0/24
		ALLOCATED,
		REGISTRY,
		153741,
		"RIPE NCC"
	},
	{
		// 213.0.0.0/24
		ALLOCATED,
		REGISTRY,
		185783,
		"RIPE NCC"
	},
	{
		// 214.0.0.0/24
		ALLOCATED,
		DIRECT,
		414,
		"US-DOD"
	},
	{
		// 215.0.0.0/24
		ALLOCATED,
		DIRECT,
		166,
		"US-DOD"
	},
	{
		// 216.0.0.0/24
		ALLOCATED,
		REGISTRY,
		119415,
		"ARIN"
	},
	{
		// 217.0.0.0/24
		ALLOCATED,
		REGISTRY,
		193939,
		"RIPE NCC"
	},
	{
		// 218.0.0.0/24
		ALLOCATED,
		REGISTRY,
		185224,
		"APNIC"
	},
	{
		// 219.0.0.0/24
		ALLOCATED,
		REGISTRY,
		122612,
		"APNIC"
	},
	{
		// 220.0.0.0/24
		ALLOCATED,
		REGISTRY,
		167570,
		"APNIC"
	},
	{
		// 221.0.0.0/24
		ALLOCATED,
		REGISTRY,
		144090,
		"APNIC"
	},
	{
		// 222.0.0.0/24
		ALLOCATED,
		REGISTRY,
		266493,
		"APNIC"
	},
	{
		// 223.0.0.0/24
		ALLOCATED,
		REGISTRY,
		1586,
		"APNIC"
	},
	{
		// 224.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 225.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 226.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 227.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 228.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 229.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 230.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 231.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 232.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 233.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 234.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 235.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 236.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 237.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 238.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 239.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 240.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 241.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 242.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 243.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 244.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 245.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 246.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 247.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 248.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 249.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 250.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 251.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 252.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 253.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 254.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	},
	{
		// 255.0.0.0/24
		RESERVED,
		EMPTY,
		0,
		NULL
	}
};

#endif

