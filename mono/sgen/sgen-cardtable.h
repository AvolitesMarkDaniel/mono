/*
 * Copyright 2001-2003 Ximian, Inc
 * Copyright 2003-2010 Novell, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __MONO_SGEN_CARD_TABLE_INLINES_H__
#define __MONO_SGEN_CARD_TABLE_INLINES_H__

/*WARNING: This function returns the number of cards regardless of overflow in case of overlapping cards.*/
mword sgen_card_table_number_of_cards_in_range (mword address, mword size);

void sgen_card_table_reset_region (mword start, mword end);
void* sgen_card_table_align_pointer (void *ptr);
void sgen_card_table_mark_range (mword address, mword size);
void sgen_cardtable_scan_object (GCObject *obj, mword obj_size, guint8 *cards,
		gboolean mod_union, ScanCopyContext ctx);

gboolean sgen_card_table_get_card_data (guint8 *dest, mword address, mword cards);

guint8* sgen_card_table_alloc_mod_union (char *obj, mword obj_size);
void sgen_card_table_free_mod_union (guint8 *mod_union, char *obj, mword obj_size);

void sgen_card_table_update_mod_union_from_cards (guint8 *dest, guint8 *start_card, size_t num_cards);
void sgen_card_table_update_mod_union (guint8 *dest, char *obj, mword obj_size, size_t *out_num_cards);

guint8* sgen_get_card_table_configuration (int *shift_bits, gpointer *mask);

void sgen_card_table_init (SgenRememberedSet *remset);

/*How many bytes a single card covers*/
#define CARD_BITS 9

/* How many bits of the address space is covered by the card table.
 * If this value is smaller than the number of address bits, card aliasing is required.
 */
#define CARD_TABLE_BITS 32

#define CARD_SIZE_IN_BYTES (1 << CARD_BITS)
#define CARD_COUNT_BITS (CARD_TABLE_BITS - CARD_BITS)
#define CARD_COUNT_IN_BYTES (1 << CARD_COUNT_BITS)
#define CARD_MASK ((1 << CARD_COUNT_BITS) - 1)

#if SIZEOF_VOID_P * 8 > CARD_TABLE_BITS
#define SGEN_HAVE_OVERLAPPING_CARDS	1
#endif

extern guint8 *sgen_cardtable;


#ifdef SGEN_HAVE_OVERLAPPING_CARDS

static inline guint8*
sgen_card_table_get_card_address (mword address)
{
	return sgen_cardtable + ((address >> CARD_BITS) & CARD_MASK);
}

extern guint8 *sgen_shadow_cardtable;

#define SGEN_SHADOW_CARDTABLE_END (sgen_shadow_cardtable + CARD_COUNT_IN_BYTES)

static inline guint8*
sgen_card_table_get_shadow_card_address (mword address)
{
	return sgen_shadow_cardtable + ((address >> CARD_BITS) & CARD_MASK);
}

static inline gboolean
sgen_card_table_card_begin_scanning (mword address)
{
	return *sgen_card_table_get_shadow_card_address (address) != 0;
}

static inline void
sgen_card_table_prepare_card_for_scanning (guint8 *card)
{
}

#define sgen_card_table_get_card_scan_address sgen_card_table_get_shadow_card_address

#else

static inline guint8*
sgen_card_table_get_card_address (mword address)
{
	return sgen_cardtable + (address >> CARD_BITS);
}

static inline gboolean
sgen_card_table_card_begin_scanning (mword address)
{
	guint8 *card = sgen_card_table_get_card_address (address);
	gboolean res = *card;
	*card = 0;
	return res;
}

static inline void
sgen_card_table_prepare_card_for_scanning (guint8 *card)
{
	*card = 0;
}

#define sgen_card_table_get_card_scan_address sgen_card_table_get_card_address

#endif

static inline gboolean
sgen_card_table_address_is_marked (mword address)
{
	return *sgen_card_table_get_card_address (address) != 0;
}

static inline void
sgen_card_table_mark_address (mword address)
{
	*sgen_card_table_get_card_address (address) = 1;
}

static inline size_t
sgen_card_table_get_card_offset (char *ptr, char *base)
{
	return (ptr - base) >> CARD_BITS;
}

#endif
