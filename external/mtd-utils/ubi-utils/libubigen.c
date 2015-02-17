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
 * Generating UBI images.
 *
 * Authors: Oliver Lohmann
 *          Artem Bityutskiy
 */

#define PROGRAM_NAME "libubigen"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <mtd/ubi-media.h>
#include <mtd/ubi-user.h>
#include <mtd_swab.h>
#include <libubigen.h>
#include <crc32.h>
#include "common.h"

void ubigen_info_init(struct ubigen_info *ui, int peb_size, int min_io_size,
		      int subpage_size, int vid_hdr_offs, int ubi_ver,
		      uint32_t image_seq)
{
	if (!vid_hdr_offs) {
		vid_hdr_offs = UBI_EC_HDR_SIZE + subpage_size - 1;
		vid_hdr_offs /= subpage_size;
		vid_hdr_offs *= subpage_size;
	}

	ui->peb_size = peb_size;
	ui->min_io_size = min_io_size;
	ui->vid_hdr_offs = vid_hdr_offs;
	ui->data_offs = vid_hdr_offs + UBI_VID_HDR_SIZE + min_io_size - 1;
	ui->data_offs /= min_io_size;
	ui->data_offs *= min_io_size;
	ui->leb_size = peb_size - ui->data_offs;
	ui->ubi_ver = ubi_ver;
	ui->image_seq = image_seq;

	ui->max_volumes = ui->leb_size / UBI_VTBL_RECORD_SIZE;
	if (ui->max_volumes > UBI_MAX_VOLUMES)
		ui->max_volumes = UBI_MAX_VOLUMES;
	ui->vtbl_size = ui->max_volumes * UBI_VTBL_RECORD_SIZE;
}

struct ubi_vtbl_record *ubigen_create_empty_vtbl(const struct ubigen_info *ui)
{
	struct ubi_vtbl_record *vtbl;
	int i;

	vtbl = calloc(1, ui->vtbl_size);
	if (!vtbl) {
		sys_errmsg("cannot allocate %d bytes of memory", ui->vtbl_size);
		return NULL;
	}

	for (i = 0; i < ui->max_volumes; i++) {
		uint32_t crc = mtd_crc32(UBI_CRC32_INIT, &vtbl[i],
				     UBI_VTBL_RECORD_SIZE_CRC);
		vtbl[i].crc = cpu_to_be32(crc);
	}

	return vtbl;
}

int ubigen_add_volume(const struct ubigen_info *ui,
		      const struct ubigen_vol_info *vi,
		      struct ubi_vtbl_record *vtbl)
{
	struct ubi_vtbl_record *vtbl_rec = &vtbl[vi->id];
	uint32_t tmp;

	if (vi->id >= ui->max_volumes) {
		errmsg("too high volume id %d, max. volumes is %d",
		       vi->id, ui->max_volumes);
		errno = EINVAL;
		return -1;
	}

	if (vi->alignment >= ui->leb_size) {
		errmsg("too large alignment %d, max is %d (LEB size)",
		       vi->alignment, ui->leb_size);
		errno = EINVAL;
		return -1;
	}

	memset(vtbl_rec, 0, sizeof(struct ubi_vtbl_record));
	tmp = (vi->bytes + ui->leb_size - 1) / ui->leb_size;
	vtbl_rec->reserved_pebs = cpu_to_be32(tmp);
	vtbl_rec->alignment = cpu_to_be32(vi->alignment);
	vtbl_rec->vol_type = vi->type;
	tmp = ui->leb_size % vi->alignment;
	vtbl_rec->data_pad = cpu_to_be32(tmp);
	vtbl_rec->flags = vi->flags;

	memcpy(vtbl_rec->name, vi->name, vi->name_len);
	vtbl_rec->name[vi->name_len] = '\0';
	vtbl_rec->name_len = cpu_to_be16(vi->name_len);

	tmp = mtd_crc32(UBI_CRC32_INIT, vtbl_rec, UBI_VTBL_RECORD_SIZE_CRC);
	vtbl_rec->crc =	 cpu_to_be32(tmp);
	return 0;
}

void ubigen_init_ec_hdr(const struct ubigen_info *ui,
		        struct ubi_ec_hdr *hdr, long long ec)
{
	uint32_t crc;

	memset(hdr, 0, sizeof(struct ubi_ec_hdr));

	hdr->magic = cpu_to_be32(UBI_EC_HDR_MAGIC);
	hdr->version = ui->ubi_ver;
	hdr->ec = cpu_to_be64(ec);
	hdr->vid_hdr_offset = cpu_to_be32(ui->vid_hdr_offs);
	hdr->data_offset = cpu_to_be32(ui->data_offs);
	hdr->image_seq = cpu_to_be32(ui->image_seq);

	crc = mtd_crc32(UBI_CRC32_INIT, hdr, UBI_EC_HDR_SIZE_CRC);
	hdr->hdr_crc = cpu_to_be32(crc);
}

void ubigen_init_vid_hdr(const struct ubigen_info *ui,
			 const struct ubigen_vol_info *vi,
			 struct ubi_vid_hdr *hdr, int lnum,
			 unsigned long long sqnum,
			 const void *data, int data_size)
{
	uint32_t crc;

	memset(hdr, 0, sizeof(struct ubi_vid_hdr));

	hdr->magic = cpu_to_be32(UBI_VID_HDR_MAGIC);
	hdr->version = ui->ubi_ver;
	hdr->vol_type = vi->type;
	hdr->vol_id = cpu_to_be32(vi->id);
	hdr->lnum = cpu_to_be32(lnum);
	hdr->data_pad = cpu_to_be32(vi->data_pad);
	hdr->compat = vi->compat;
	hdr->sqnum = cpu_to_be64(sqnum);

	if (vi->type == UBI_VID_STATIC) {
		hdr->data_size = cpu_to_be32(data_size);
		hdr->used_ebs = cpu_to_be32(vi->used_ebs);
		crc = mtd_crc32(UBI_CRC32_INIT, data, data_size);
		hdr->data_crc = cpu_to_be32(crc);
	}

	crc = mtd_crc32(UBI_CRC32_INIT, hdr, UBI_VID_HDR_SIZE_CRC);
	hdr->hdr_crc = cpu_to_be32(crc);
}

int ubigen_write_volume(const struct ubigen_info *ui,
			const struct ubigen_vol_info *vi, long long ec,
			long long bytes, int in, int out, int *out_lnum)
{
	int len = vi->usable_leb_size, rd, lnum = 0;
	char *inbuf, *outbuf;

	if (vi->id >= ui->max_volumes) {
		errmsg("too high volume id %d, max. volumes is %d",
		       vi->id, ui->max_volumes);
		errno = EINVAL;
		return -1;
	}

	if (vi->alignment >= ui->leb_size) {
		errmsg("too large alignment %d, max is %d (LEB size)",
		       vi->alignment, ui->leb_size);
		errno = EINVAL;
		return -1;
	}

	inbuf = malloc(ui->leb_size);
	if (!inbuf)
		return sys_errmsg("cannot allocate %d bytes of memory",
				  ui->leb_size);
	outbuf = malloc(ui->peb_size);
	if (!outbuf) {
		sys_errmsg("cannot allocate %d bytes of memory", ui->peb_size);
		goto out_free;
	}

	memset(outbuf, 0xFF, ui->data_offs);
	ubigen_init_ec_hdr(ui, (struct ubi_ec_hdr *)outbuf, ec);

	while (bytes) {
		int l;
		struct ubi_vid_hdr *vid_hdr;

		if (bytes < len)
			len = bytes;
		bytes -= len;

		l = len;
		do {
			rd = read(in, inbuf + len - l, l);
			if (rd != l) {
				sys_errmsg("cannot read %d bytes from the input file", l);
				goto out_free1;
			}

			l -= rd;
		} while (l);

		vid_hdr = (struct ubi_vid_hdr *)(&outbuf[ui->vid_hdr_offs]);
		ubigen_init_vid_hdr(ui, vi, vid_hdr, lnum, 0, inbuf, len);

		memcpy(outbuf + ui->data_offs, inbuf, len);
		memset(outbuf + ui->data_offs + len, 0xFF,
		       ui->peb_size - ui->data_offs - len);

		if (write(out, outbuf, ui->peb_size) != ui->peb_size) {
			sys_errmsg("cannot write %d bytes to the output file", ui->peb_size);
			goto out_free1;
		}

		lnum += 1;
	}
	*out_lnum = lnum; //MTK: output used LEB

	free(outbuf);
	free(inbuf);
	return 0;

out_free1:
	free(outbuf);
out_free:
	free(inbuf);
	return -1;
}

int ubigen_write_layout_vol(const struct ubigen_info *ui, int peb1, int peb2,
			    long long ec1, long long ec2,
			    struct ubi_vtbl_record *vtbl, int fd)
{
	int ret;
	struct ubigen_vol_info vi;
	char *outbuf;
	struct ubi_vid_hdr *vid_hdr;
	off_t seek;

	vi.bytes = ui->leb_size * UBI_LAYOUT_VOLUME_EBS;
	vi.id = UBI_LAYOUT_VOLUME_ID;
	vi.alignment = UBI_LAYOUT_VOLUME_ALIGN;
	vi.data_pad = ui->leb_size % UBI_LAYOUT_VOLUME_ALIGN;
	vi.usable_leb_size = ui->leb_size - vi.data_pad;
	vi.data_pad = ui->leb_size - vi.usable_leb_size;
	vi.type = UBI_LAYOUT_VOLUME_TYPE;
	vi.name = UBI_LAYOUT_VOLUME_NAME;
	vi.name_len = strlen(UBI_LAYOUT_VOLUME_NAME);
	vi.compat = UBI_LAYOUT_VOLUME_COMPAT;

	outbuf = malloc(ui->peb_size);
	if (!outbuf)
		return sys_errmsg("failed to allocate %d bytes",
				  ui->peb_size);

	memset(outbuf, 0xFF, ui->data_offs);
	vid_hdr = (struct ubi_vid_hdr *)(&outbuf[ui->vid_hdr_offs]);
	memcpy(outbuf + ui->data_offs, vtbl, ui->vtbl_size);
	memset(outbuf + ui->data_offs + ui->vtbl_size, 0xFF,
	       ui->peb_size - ui->data_offs - ui->vtbl_size);

	seek = peb1 * ui->peb_size;
	if (lseek(fd, seek, SEEK_SET) != seek) {
		sys_errmsg("cannot seek output file");
		goto out_free;
	}

	ubigen_init_ec_hdr(ui, (struct ubi_ec_hdr *)outbuf, ec1);
	ubigen_init_vid_hdr(ui, &vi, vid_hdr, 0, 0, NULL, 0);
	ret = write(fd, outbuf, ui->peb_size);
	if (ret != ui->peb_size) {
		sys_errmsg("cannot write %d bytes", ui->peb_size);
		goto out_free;
	}

	seek = peb2 * ui->peb_size;
	if (lseek(fd, seek, SEEK_SET) != seek) {
		sys_errmsg("cannot seek output file");
		goto out_free;
	}
	ubigen_init_ec_hdr(ui, (struct ubi_ec_hdr *)outbuf, ec2);
	ubigen_init_vid_hdr(ui, &vi, vid_hdr, 1, 0, NULL, 0);
	ret = write(fd, outbuf, ui->peb_size);
	if (ret != ui->peb_size) {
		sys_errmsg("cannot write %d bytes", ui->peb_size);
		goto out_free;
	}

	free(outbuf);
	return 0;

out_free:
	free(outbuf);
	return -1;
}

#ifdef MTK_NAND_UBIFS_FASTMAP_SUPPORT
#define roundup(x, y) (                                 \
{                                                       \
        const typeof(y) __y = y;                        \
        (((x) + (__y - 1)) / __y) * __y;                \
}  							\
)
/**
 * ubi_calc_fm_size - calculates the fastmap size in bytes for an UBI device.
 * @ubi: UBI device description object
 */
size_t ubi_calc_fm_size(struct ubigen_info *ui)
{
        size_t size;

        size = sizeof(struct ubi_fm_hdr) + \
                sizeof(struct ubi_fm_scan_pool) + \
                sizeof(struct ubi_fm_scan_pool) + \
                (ui->peb_count * sizeof(struct ubi_fm_ec)) + \
                (sizeof(struct ubi_fm_eba) + \
                (ui->peb_count * sizeof(__be32))) + \
                sizeof(struct ubi_fm_volhdr) * UBI_MAX_VOLUMES;
        return roundup(size, ui->leb_size);
}

/**
 * ubi_write_fastmap - writes a fastmap.
 * @ubi: UBI device object
 * @new_fm: the to be written fastmap
 *
 * Returns 0 on success, < 0 indicates an internal error.
 */
static int ubi_write_fastmap(struct ubigen_info *ubi,
			     struct ubi_fastmap_layout *new_fm, 
			     struct ubigen_vol_info *vi, 
			     int vc,
			     int out_fd)
{
	size_t fm_pos = 0;
	char *fm_raw;
	struct ubi_fm_sb *fmsb;
	struct ubi_fm_hdr *fmh;
	struct ubi_fm_scan_pool *fmpl1, *fmpl2;
	struct ubi_fm_ec *fec;
	struct ubi_fm_volhdr *fvh;
	struct ubi_fm_eba *feba;
	struct ubigen_vol_info *vol;
	struct ubi_vid_hdr *vid_hdr;
	int ret, i, j, free_peb_count, used_peb_count, vol_count;
	int scrub_peb_count, erase_peb_count;
	struct ubigen_vol_info fm_vi;
	int free_start;

	memset(&fm_vi, 0, sizeof(struct ubigen_vol_info));

        fm_vi.id = UBI_FM_SB_VOLUME_ID;
        fm_vi.type = UBI_VID_DYNAMIC;
        fm_vi.compat = UBI_COMPAT_DELETE;


	fm_raw = malloc(ubi->peb_size);
	memset(fm_raw, 0, ubi->peb_size);

	ubigen_init_ec_hdr(ubi, (struct ubi_ec_hdr *)fm_raw, ubi->ec);

	vid_hdr = (struct ubi_vid_hdr *)(fm_raw + ubi->vid_hdr_offs);
	ubigen_init_vid_hdr(ubi, &fm_vi, vid_hdr, 0, 1, NULL, 0);
	fm_pos += ubi->data_offs;

	fmsb = (struct ubi_fm_sb *)(fm_raw + fm_pos);
	fm_pos += sizeof(*fmsb);

	fmh = (struct ubi_fm_hdr *)(fm_raw + fm_pos);
	fm_pos += sizeof(*fmh);

	fmsb->magic = cpu_to_be32(UBI_FM_SB_MAGIC);
	fmsb->version = UBI_FM_FMT_VERSION;
	fmsb->used_blocks = cpu_to_be32(new_fm->used_blocks);

	fmh->magic = cpu_to_be32(UBI_FM_HDR_MAGIC);
	free_peb_count = 0;
	used_peb_count = 0;
	scrub_peb_count = 0;
	erase_peb_count = 0;
	vol_count = 0;

	fmpl1 = (struct ubi_fm_scan_pool *)(fm_raw + fm_pos);
	fm_pos += sizeof(*fmpl1);
	fmpl1->magic = cpu_to_be32(UBI_FM_POOL_MAGIC);
	fmpl1->size = cpu_to_be16(ubi->fm_pool.size);
	fmpl1->max_size = cpu_to_be16(ubi->fm_pool.max_size);

	for (i = 0; i < ubi->fm_pool.size; i++)
		fmpl1->pebs[i] = cpu_to_be32(ubi->fm_pool.pebs[i]);

	fmpl2 = (struct ubi_fm_scan_pool *)(fm_raw + fm_pos);
	fm_pos += sizeof(*fmpl2);
	fmpl2->magic = cpu_to_be32(UBI_FM_POOL_MAGIC);
	fmpl2->size = cpu_to_be16(ubi->fm_wl_pool.size);
	fmpl2->max_size = cpu_to_be16(ubi->fm_wl_pool.max_size);

	for (i = 0; i < ubi->fm_wl_pool.size; i++)
		fmpl2->pebs[i] = cpu_to_be32(ubi->fm_wl_pool.pebs[i]);

	free_start = ubi->used_peb + ubi->fm_pool.size + ubi->fm_wl_pool.size;

	fmh->free_peb_count = cpu_to_be32(free_peb_count); //0
	/* used list*/
	for (i = 0 ; i < ubi->used_peb; i++) {
		if(i == 2) //Skip PEB 2
			continue;
		fec = (struct ubi_fm_ec *)(fm_raw + fm_pos);

		fec->pnum = cpu_to_be32(i);
		fec->ec = cpu_to_be32(ubi->ec);

		used_peb_count++;
		fm_pos += sizeof(*fec);
	}
	fmh->used_peb_count = cpu_to_be32(used_peb_count);

	fmh->scrub_peb_count = cpu_to_be32(scrub_peb_count); //0
	/* empty pebs to erase*/
	for (i = free_start ; i < ubi->peb_count; i++) {
		fec = (struct ubi_fm_ec *)(fm_raw + fm_pos);

		fec->pnum = cpu_to_be32(i);
		fec->ec = cpu_to_be32(ubi->ec);

		erase_peb_count++;
		fm_pos += sizeof(*fec);
	}
	fmh->erase_peb_count = cpu_to_be32(erase_peb_count);

	for (i = 0; i < vc; i++) {
		int32_t tmp, reserved_pebs;
		vol = &vi[i];

		if (vol->bytes == 0)
			continue;

		vol_count++;

		fvh = (struct ubi_fm_volhdr *)(fm_raw + fm_pos);
		fm_pos += sizeof(*fvh);

		fvh->magic = cpu_to_be32(UBI_FM_VHDR_MAGIC);
		fvh->vol_id = cpu_to_be32(vol->id);
		fvh->vol_type = UBI_DYNAMIC_VOLUME;
		reserved_pebs = (vol->bytes + ubi->leb_size - 1) / ubi->leb_size;
		fvh->used_ebs = cpu_to_be32(reserved_pebs);
		tmp = ubi->leb_size % vol->alignment;
		fvh->data_pad = cpu_to_be32(tmp);
		fvh->last_eb_bytes = cpu_to_be32(vol->usable_leb_size);


		feba = (struct ubi_fm_eba *)(fm_raw + fm_pos);
		fm_pos += sizeof(*feba) + (sizeof(__be32) * reserved_pebs);

		printf("vol %d reserved_pebs %d\n", i, reserved_pebs);
		printf("vol %d start %d stop %d\n", i, vol->start, vol->stop);
		for (j = 0; j < reserved_pebs; j++) {
			if(vol->start > 0 && j < vol->stop - vol->start) 
				feba->pnum[j] = cpu_to_be32(vol->start+j);
			else
				feba->pnum[j] = cpu_to_be32(-1);
		}

		feba->reserved_pebs = cpu_to_be32(reserved_pebs);
		feba->magic = cpu_to_be32(UBI_FM_EBA_MAGIC);
	}
	//for vol table 
	{
		uint32_t tmp;

		vol_count++;

		fvh = (struct ubi_fm_volhdr *)(fm_raw + fm_pos);
		fm_pos += sizeof(*fvh);

		fvh->magic = cpu_to_be32(UBI_FM_VHDR_MAGIC);
		fvh->vol_id = cpu_to_be32(UBI_LAYOUT_VOLUME_ID);
		fvh->vol_type = UBI_DYNAMIC_VOLUME;
		fvh->used_ebs = cpu_to_be32(UBI_LAYOUT_VOLUME_EBS);
		tmp = ubi->leb_size % UBI_LAYOUT_VOLUME_ALIGN;
		fvh->data_pad = cpu_to_be32(tmp);
		fvh->last_eb_bytes = cpu_to_be32(UBI_LAYOUT_VOLUME_EBS);


		feba = (struct ubi_fm_eba *)(fm_raw + fm_pos);
		fm_pos += sizeof(*feba) + (sizeof(__be32) * UBI_LAYOUT_VOLUME_EBS);

		for (j = 0; j < UBI_LAYOUT_VOLUME_EBS; j++)
			feba->pnum[j] = cpu_to_be32(j);

		feba->reserved_pebs = cpu_to_be32(UBI_LAYOUT_VOLUME_EBS);
		feba->magic = cpu_to_be32(UBI_FM_EBA_MAGIC);
	}
	fmh->vol_count = cpu_to_be32(vol_count);
	fmh->bad_peb_count = cpu_to_be32(0);

	vid_hdr->lnum = 0;


	for (i = 0; i < new_fm->used_blocks; i++) {
		fmsb->block_loc[i] = cpu_to_be32(new_fm->e[i]->pnum);
		fmsb->block_ec[i] = cpu_to_be32(new_fm->e[i]->ec);
	}

	fmsb->data_crc = 0;
	fmsb->data_crc = cpu_to_be32(mtd_crc32(UBI_CRC32_INIT, fmsb,
					   ubi->fm_size));

	for (i = 0; i < new_fm->used_blocks; i++) {
		off_t seek = ubi->peb_size*(2+i);
		printf("writing fastmap SB to PEB %i\n", new_fm->e[i]->pnum);
		if(lseek(out_fd, seek, SEEK_SET) != seek) {
			sys_errmsg("cannot seek output file");
			goto out_free;
		}
		ret = write(out_fd, fm_raw, ubi->peb_size);
		if (ret != ubi->peb_size) {
			sys_errmsg("cannot write %d bytes", ubi->peb_size);
			goto out_free;
		} else 
			ret = 0;
	}

	printf("fastmap written!");

out_free:
	free(fm_raw);

	return ret;
}


/**
 * refill_wl_pool - refills all the fastmap pool used by the
 * WL sub-system and ubi_wl_get_peb.
 * @ubi: UBI device description object
 */
static void ubi_refill_pools(struct ubigen_info *ui, int fd)
{
        struct ubi_fm_pool *pool = &ui->fm_wl_pool;
	int start = ui->used_peb;
	off_t seek;
	char *outbuf;

	outbuf = malloc(ui->peb_size);
	memset(outbuf, 0xFF, ui->peb_size);
	ubigen_init_ec_hdr(ui, (struct ubi_ec_hdr *)outbuf, ui->ec);

        printf("Start PEB %d for wl pool size %d\n", start, pool->max_size);
        for (pool->size = 0; pool->size < pool->max_size; pool->size++) {
                if (ui->peb_count - start < 5)
                        break;

		seek = start * ui->peb_size;
		if (lseek(fd, seek, SEEK_SET) != seek) {
			sys_errmsg("cannot seek output file");
			break;
		}
		if (write(fd, outbuf, ui->peb_size) != ui->peb_size) {
			sys_errmsg("cannot write %d bytes to the output file", ui->peb_size);
			break;
		}

                pool->pebs[pool->size] = start;
		start++;
        }
        printf( "End PEB %d for wl pool\n", start-1);
        printf( "found %d PEB for wl pool\n", pool->size);
        pool->used = 0;

        pool = &ui->fm_pool;

        printf( "Start PEB %d for user wl pool size %d\n", start, pool->max_size);
        for (pool->size = 0; pool->size < pool->max_size; pool->size++) {
                if (ui->peb_count - start < 1)
                        break;

		seek = start * ui->peb_size;
		if (lseek(fd, seek, SEEK_SET) != seek) {
			sys_errmsg("cannot seek output file");
			break;
		}
		if (write(fd, outbuf, ui->peb_size) != ui->peb_size) {
			sys_errmsg("cannot write %d bytes to the output file", ui->peb_size);
			break;
		}

                pool->pebs[pool->size] = start;
		start++;
        }
        printf( "End PEB %d for user wl pool\n", start-1);
        printf( "found %d PEB for wl pool\n", pool->size);
        pool->used = 0;
}


/**
 * ubi_update_fastmap - will be called by UBI if a volume changes or
 * a fastmap pool becomes full.
 * @ubi: UBI device object
 *
 * Returns 0 on success, < 0 indicates an internal error.
 */
int ubi_update_fastmap(struct ubigen_info *ubi, struct ubigen_vol_info *vi, int vol_count, int out_fd)
{
	int ret, i;
	struct ubi_fastmap_layout *new_fm;

	ubi_refill_pools(ubi, out_fd);

	new_fm = malloc(sizeof(*new_fm));
	if (!new_fm) {
		return -ENOMEM;
	}
	memset(new_fm, 0, sizeof(*new_fm));

	new_fm->used_blocks = ubi->fm_size / ubi->leb_size;
	printf("new_fm->used_blocks %d (we support only 1 block now)\n", new_fm->used_blocks);

	for (i = 0; i < new_fm->used_blocks; i++) {
		new_fm->e[i] = malloc(sizeof(struct ubi_wl_entry));
		if (!new_fm->e[i]) {
			while (i--)
				free(new_fm->e[i]);

			free(new_fm);
			return -ENOMEM;
		}
		memset(new_fm->e[i], 0, sizeof(struct ubi_wl_entry));
	}

	if (new_fm->used_blocks > UBI_FM_MAX_BLOCKS) {
		printf("fastmap too large");
		ret = -ENOSPC;
		goto err;
	}

	new_fm->e[0]->pnum = 2; //2 or 3
	new_fm->e[0]->ec = ubi->ec;

	ret = ubi_write_fastmap(ubi, new_fm, vi, vol_count, out_fd);

	if (ret)
		goto err;

out_unlock:
	return ret;

err:
	free(new_fm);
	printf("Unable to write new fastmap, err=%i", ret);
	ret = 0;
	goto out_unlock;
}
#endif
