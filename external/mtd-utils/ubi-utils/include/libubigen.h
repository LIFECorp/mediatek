/*
 * Copyright (c) International Business Machines Corp., 2006
 * Copyright (C) 2008 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Authors: Frank Haverkamp
 *          Artem Bityutskiy
 */

#ifndef __LIBUBIGEN_H__
#define __LIBUBIGEN_H__

#include <stdint.h>
#include <mtd/ubi-media.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MTK_NAND_UBIFS_FASTMAP_SUPPORT 

/**
 * struct ubi_wl_entry - wear-leveling entry.
 * @ec: erase counter
 * @pnum: physical eraseblock number
 *
 * This data structure is used in the WL sub-system. Each physical eraseblock
 * has a corresponding &struct wl_entry object which may be kept in different
 * RB-trees. See WL sub-system for details.
 */
struct ubi_wl_entry {
        int ec;
        int pnum;
};

/**
 * struct ubi_fastmap_layout - in-memory fastmap data structure.
 * @e: PEBs used by the current fastmap
 * @to_be_tortured: if non-zero tortured this PEB
 * @used_blocks: number of used PEBs
 * @max_pool_size: maximal size of the user pool
 * @max_wl_pool_size: maximal size of the pool used by the WL sub-system
 */
struct ubi_fastmap_layout {
        struct ubi_wl_entry *e[UBI_FM_MAX_BLOCKS];
        int to_be_tortured[UBI_FM_MAX_BLOCKS];
        int used_blocks;
        int max_pool_size;
        int max_wl_pool_size;
};

/**
 * struct ubi_fm_pool - in-memory fastmap pool
 * @pebs: PEBs in this pool
 * @used: number of used PEBs
 * @size: total number of PEBs in this pool
 * @max_size: maximal size of the pool
 *
 * A pool gets filled with up to max_size.
 * If all PEBs within the pool are used a new fastmap will be written
 * to the flash and the pool gets refilled with empty PEBs.
 *
 */
struct ubi_fm_pool {
        int pebs[UBI_FM_MAX_POOL_SIZE];
        int used;
        int size;
        int max_size;
};
#endif

/**
 * struct ubigen_info - libubigen information.
 * @leb_size: logical eraseblock size
 * @peb_size: size of the physical eraseblock
 * @min_io_size: minimum input/output unit size
 * @vid_hdr_offs: offset of the VID header
 * @data_offs: data offset
 * @ubi_ver: UBI version
 * @vtbl_size: volume table size
 * @max_volumes: maximum amount of volumes
 * @image_seq: UBI image sequence number
 */
struct ubigen_info
{
	int leb_size;
	int peb_size;
	int min_io_size;
	int vid_hdr_offs;
	int data_offs;
	int ubi_ver;
	int vtbl_size;
	int max_volumes;
	uint32_t image_seq;

	long long ec; //MTK: args.ec
	int device_size;
	int peb_count;
	int used_peb;
#ifdef MTK_NAND_UBIFS_FASTMAP_SUPPORT 
	size_t fm_size;
	struct ubi_fm_pool fm_pool;
	struct ubi_fm_pool fm_wl_pool;
#endif
};

/**
 * struct ubigen_vol_info - information about a volume.
 * @id: volume id
 * @type: volume type (%UBI_VID_DYNAMIC or %UBI_VID_STATIC)
 * @alignment: volume alignment
 * @data_pad: how many bytes are unused at the end of the each physical
 *            eraseblock to satisfy the requested alignment
 * @usable_leb_size: LEB size accessible for volume users
 * @name: volume name
 * @name_len: volume name length
 * @compat: compatibility of this volume (%0, %UBI_COMPAT_DELETE,
 *          %UBI_COMPAT_IGNORE, %UBI_COMPAT_PRESERVE, or %UBI_COMPAT_REJECT)
 * @used_ebs: total number of used logical eraseblocks in this volume (relevant
 *            for static volumes only)
 * @bytes: size of the volume contents in bytes (relevant for static volumes
 *         only)
 * @flags: volume flags (%UBI_VTBL_AUTORESIZE_FLG)
 */
struct ubigen_vol_info
{
	int id;
	int type;
	int alignment;
	int data_pad;
	int usable_leb_size;
	const char *name;
	int name_len;
	int compat;
	int used_ebs;
	long long bytes;
	uint8_t flags;

	int start, stop; //MTK: start_peb, stop_peb
};

/**
 * ubigen_info_init - initialize libubigen.
 * @ui: libubigen information
 * @peb_size: flash physical eraseblock size
 * @min_io_size: flash minimum input/output unit size
 * @subpage_size: flash sub-page, if present (has to be equivalent to
 *                @min_io_size if does not exist)
 * @vid_hdr_offs: offset of the VID header
 * @ubi_ver: UBI version
 * @image_seq: UBI image sequence number
 */
void ubigen_info_init(struct ubigen_info *ui, int peb_size, int min_io_size,
		      int subpage_size, int vid_hdr_offs, int ubi_ver,
		      uint32_t image_seq);

/**
 * ubigen_create_empty_vtbl - creates empty volume table.
 * @ui: libubigen information
 *
 * This function creates an empty volume table and returns a pointer to it in
 * case of success and %NULL in case of failure. The returned object has to be
 * freed with 'free()' call.
 */
struct ubi_vtbl_record *ubigen_create_empty_vtbl(const struct ubigen_info *ui);

/**
 * ubigen_init_ec_hdr - initialize EC header.
 * @ui: libubigen information
 * @hdr: the EC header to initialize
 * @ec: erase counter value
 */
void ubigen_init_ec_hdr(const struct ubigen_info *ui,
		        struct ubi_ec_hdr *hdr, long long ec);

/**
 * ubigen_init_vid_hdr - initialize VID header.
 * @ui: libubigen information
 * @vi: volume information
 * @hdr: the VID header to initialize
 * @lnum: logical eraseblock number
 * @data: the contents of the LEB (static volumes only)
 * @data_size: amount of data in this LEB (static volumes only)
 *
 * Note, @used_ebs, @data and @data_size are ignored in case of dynamic
 * volumes.
 */
void ubigen_init_vid_hdr(const struct ubigen_info *ui,
			 const struct ubigen_vol_info *vi,
			 struct ubi_vid_hdr *hdr, int lnum,
			 unsigned long long sqnum,
			 const void *data, int data_size);

/**
 * ubigen_add_volume - add a volume to the volume table.
 * @ui: libubigen information
 * @vi: volume information
 * @vtbl: volume table to add to
 *
 * This function adds volume described by input parameters to the volume table
 * @vtbl.
 */
int ubigen_add_volume(const struct ubigen_info *ui,
		      const struct ubigen_vol_info *vi,
		      struct ubi_vtbl_record *vtbl);

/**
 * ubigen_write_volume - write UBI volume.
 * @ui: libubigen information
 * @vi: volume information
 * @ec: erase counter value to put to EC headers
 * @bytes: volume size in bytes
 * @in: input file descriptor (has to be properly seeked)
 * @out: output file descriptor
 *
 * This function reads the contents of the volume from the input file @in and
 * writes the UBI volume to the output file @out. Returns zero on success and
 * %-1 on failure.
 */
//MTK: add output lnum
int ubigen_write_volume(const struct ubigen_info *ui,
			const struct ubigen_vol_info *vi, long long ec,
			long long bytes, int in, int out, int *lnum);

/**
 * ubigen_write_layout_vol - write UBI layout volume
 * @ui: libubigen information
 * @peb1: physical eraseblock number to write the first volume table copy
 * @peb2: physical eraseblock number to write the second volume table copy
 * @ec1: erase counter value for @peb1
 * @ec2: erase counter value for @peb1
 * @vtbl: volume table
 * @fd: output file descriptor seeked to the proper position
 *
 * This function creates the UBI layout volume which contains 2 copies of the
 * volume table. Returns zero in case of success and %-1 in case of failure.
 */
int ubigen_write_layout_vol(const struct ubigen_info *ui, int peb1, int peb2,
			    long long ec1, long long ec2,
			    struct ubi_vtbl_record *vtbl, int fd);

#ifdef MTK_NAND_UBIFS_FASTMAP_SUPPORT 
size_t ubi_calc_fm_size(struct ubigen_info *ui);
int ubi_update_fastmap(struct ubigen_info *ubi, struct ubigen_vol_info *vi, int vol_count, int out_fd);
#endif

#ifdef __cplusplus
}
#endif

#endif /* !__LIBUBIGEN_H__ */
