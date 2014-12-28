/*-
* Copyright (c) 1990, 1993
*      The Regents of the University of California.  All rights reserved.
*
* This code is derived from software contributed to Berkeley by
* Chris Torek.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. All advertising materials mentioning features or use of this software
*    must display the following acknowledgement:
*      This product includes software developed by the University of
*      California, Berkeley and its contributors.
* 4. Neither the name of the University nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*
* From: Id: vfscanf.c,v 1.13 1998/09/25 12:20:27 obrien Exp
* From: static char sccsid[] = "@(#)strtol.c   8.1 (Berkeley) 6/4/93";
* From: static char sccsid[] = "@(#)strtoul.c  8.1 (Berkeley) 6/4/93";
*/

#include <esp_common.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// Add some functions missing from ESP library

#define BUF             32      /* Maximum length of numeric string. */

/*
* Flags used during conversion.
*/
#define LONG            0x01    /* l: long or double */
#define SHORT           0x04    /* h: short */
#define SUPPRESS        0x08    /* suppress assignment */
#define POINTER         0x10    /* weird %p pointer (`fake hex') */
#define NOSKIP          0x20    /* do not skip blanks */
#define QUAD            0x400

/*
* The following are used in numeric conversions only:
* SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
* SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.
*/
#define SIGNOK          0x40    /* +/- is (still) legal */
#define NDIGITS         0x80    /* no digits detected */

#define DPTOK           0x100   /* (float) decimal point is still legal */
#define EXPOK           0x200   /* (float) exponent (e+3, etc) still legal */

#define PFXOK           0x100   /* 0x prefix is (still) legal */
#define NZDIGITS        0x200   /* no zero digits detected */

/*
* Conversion types.
*/
#define CT_CHAR         0       /* %c conversion */
#define CT_CCL          1       /* %[...] conversion */
#define CT_STRING       2       /* %s conversion */
#define CT_INT          3       /* integer, i.e., strtoq or strtouq */


#define QUAD_MIN -9223372036854775807
#define QUAD_MAX 9223372036854775807
#define UQUAD_MAX 0xffffffffffffffff


typedef unsigned char u_char;
typedef long long quad_t;
typedef unsigned long long u_quad_t;
typedef unsigned uint_t;
typedef unsigned * uintptr_t;


typedef u_quad_t(*ccfntype) (const char *, char **, int);

#define MAXLN 256

/*
* This array is designed for mapping upper and lower case letter
* together for a case independent comparison.  The mappings are
* based upon ascii character sequences.
*/
static const u_char charmap[] = {
	'\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
	'\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
	'\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
	'\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
	'\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
	'\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
	'\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
	'\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
	'\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
	'\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
	'\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
	'\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
	'\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
	'\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
	'\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
	'\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
	'\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
	'\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
	'\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
	'\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
	'\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
	'\300', '\301', '\302', '\303', '\304', '\305', '\306', '\307',
	'\310', '\311', '\312', '\313', '\314', '\315', '\316', '\317',
	'\320', '\321', '\322', '\323', '\324', '\325', '\326', '\327',
	'\330', '\331', '\332', '\333', '\334', '\335', '\336', '\337',
	'\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
	'\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
	'\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
	'\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
	};

int ICACHE_FLASH_ATTR strcasecmp(const char *s1, const char *s2) {
	register const u_char *cm = charmap,
		*us1 = (const u_char *)s1,
		*us2 = (const u_char *)s2;

	while (cm[*us1] == cm[*us2++])
		if (*us1++ == '\0')
			return (0);
	return (cm[*us1] - cm[*--us2]);
	}

int ICACHE_FLASH_ATTR strncasecmp(const char *s1, const char *s2, size_t n) {
	if (n != 0) {
		register const u_char *cm = charmap,
			*us1 = (const u_char *)s1,
			*us2 = (const u_char *)s2;

		do {
			if (cm[*us1] != cm[*us2++])
				return (cm[*us1] - cm[*--us2]);
			if (*us1++ == '\0')
				break;
			} while (--n != 0);
		}
	return (0);
	}

/*
* Copy src to string dst of size siz.  At most siz-1 characters
* will be copied.  Always NUL terminates (unless siz == 0).
* Returns strlen(src); if retval >= siz, truncation occurred.
*/
size_t ICACHE_FLASH_ATTR strlcpy(char *dst, const char *src, size_t siz) {
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
			} while (--n != 0);
		}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
		}

	return(s - src - 1);	/* count does not include NUL */
	}


#define isalpha(x) (((x) >= 'A' && (x) <= 'Z') || ((x) >= 'a' && (x) <= 'z'))
#define isupper(x) ((x) >= 'A' && (x) <= 'Z')
#define isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\12')

/*
* Convert a string to an unsigned quad integer.
*
* Ignores `locale' stuff.  Assumes that the upper and lower case
* alphabets and digits are each contiguous.
*/

u_quad_t ICACHE_FLASH_ATTR strtouq(const char *nptr, char **endptr, int base) {
	const char *s = nptr;
	u_quad_t acc;
	unsigned char c;
	u_quad_t qbase, cutoff;
	int neg, any, cutlim;

	/*
	* See strtoq for comments as to the logic used.
	*/
	do {
		c = *s++;

		} while (isspace(c));
		if (c == '-') {
			neg = 1;
			c = *s++;

			}
		else {
			neg = 0;
			if (c == '+')
				c = *s++;

			}
		if ((base == 0 || base == 16) &&
			c == '0' && (*s == 'x' || *s == 'X')) {
			c = s[1];
			s += 2;
			base = 16;

			}
		if (base == 0)
			base = c == '0' ? 8 : 10;
		qbase = (unsigned)base;
		cutoff = (u_quad_t)UQUAD_MAX / qbase;
		cutlim = (u_quad_t)UQUAD_MAX % qbase;
		for (acc = 0, any = 0;; c = *s++) {
			if (!isascii(c))
				break;
			if (isdigit(c))
				c -= '0';
			else if (isalpha(c))
				c -= isupper(c) ? 'A' - 10 : 'a' - 10;
			else
				break;
			if (c >= base)
				break;
			if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
				any = -1;
			else {
				any = 1;
				acc *= qbase;
				acc += c;

				}

			}
		if (any < 0) {
			acc = UQUAD_MAX;

			}
		else if (neg)
			acc = -acc;
		if (endptr != 0)
			* ((const char **)endptr) = any ? s - 1 : nptr;
		return (acc);
	}

quad_t ICACHE_FLASH_ATTR strtoq(const char *nptr, char **endptr, int base) {
	const char *s;
	u_quad_t acc;
	unsigned char c;
	u_quad_t qbase, cutoff;
	int neg, any, cutlim;

	/*
	* Skip white space and pick up leading +/- sign if any.
	* If base is 0, allow 0x for hex and 0 for octal, else
	* assume decimal; if base is already 16, allow 0x.
	*/
	s = nptr;
	do {
		c = *s++;

		} while (isspace(c));
		if (c == '-') {
			neg = 1;
			c = *s++;

			}
		else {
			neg = 0;
			if (c == '+')
				c = *s++;

			}
		if ((base == 0 || base == 16) &&
			c == '0' && (*s == 'x' || *s == 'X')) {
			c = s[1];
			s += 2;
			base = 16;

			}
		if (base == 0)
			base = c == '0' ? 8 : 10;

		/*
		* Compute the cutoff value between legal numbers and illegal
		* numbers.  That is the largest legal value, divided by the
		* base.  An input number that is greater than this value, if
		* followed by a legal input character, is too big.  One that
		* is equal to this value may be valid or not; the limit
		* between valid and invalid numbers is then based on the last
		* digit.  For instance, if the range for quads is
		* [-9223372036854775808..9223372036854775807] and the input base
		* is 10, cutoff will be set to 922337203685477580 and cutlim to
		* either 7 (neg==0) or 8 (neg==1), meaning that if we have
		* accumulated a value > 922337203685477580, or equal but the
		* next digit is > 7 (or 8), the number is too big, and we will
		* return a range error.
		*
		* Set any if any `digits' consumed; make it negative to indicate
		* overflow.
		*/
		qbase = (unsigned)base;
		cutoff = QUAD_MAX;
		cutlim = cutoff % qbase;
		cutoff /= qbase;
		for (acc = 0, any = 0;; c = *s++) {
			if (!isascii(c))
				break;
			if (isdigit(c))
				c -= '0';
			else if (isalpha(c))
				c -= isupper(c) ? 'A' - 10 : 'a' - 10;
			else
				break;
			if (c >= base)
				break;
			if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
				any = -1;
			else {
				any = 1;
				acc *= qbase;
				acc += c;

				}

			}
		if (any < 0) {
			acc = neg ? QUAD_MIN : QUAD_MAX;

			}
		else if (neg)
			acc = -acc;
		if (endptr != 0)
			* ((const char **)endptr) = any ? s - 1 : nptr;
		return (acc);
	}



/*
* Fill in the given table from the scanset at the given format
* (just after `[').  Return a pointer to the character past the
* closing `]'.  The table has a 1 wherever characters should be
* considered part of the scanset.
*/
static const u_char *  ICACHE_FLASH_ATTR __sccl(char *tab, const u_char *fmt) {
	int c, n, v;

	/* first `clear' the whole table */
	c = *fmt++;             /* first char hat => negated scanset */
	if (c == '^') {
		v = 1;          /* default => accept */
		c = *fmt++;     /* get new first char */

		}
	else
		v = 0;          /* default => reject */

	/* XXX: Will not work if sizeof(tab*) > sizeof(char) */
	for (n = 0; n < 256; n++)
		tab[n] = v;        /* memset(tab, v, 256) */

	if (c == 0)
		return (fmt - 1);/* format ended before closing ] */

	/*
	* Now set the entries corresponding to the actual scanset
	* to the opposite of the above.
	*
	* The first character may be ']' (or '-') without being special;
	* the last character may be '-'.
	*/
	v = 1 - v;
	for (;;) {
		tab[c] = v;             /* take character c */
	doswitch:
		n = *fmt++;             /* and examine the next */
		switch (n) {

			case 0:                 /* format ended too soon */
				return (fmt - 1);

			case '-':
				/*
				* A scanset of the form
				*      [01+-]
				* is defined as `the digit 0, the digit 1,
				* the character +, the character -', but
				* the effect of a scanset such as
				*      [a-zA-Z0-9]
				* is implementation defined.  The V7 Unix
				* scanf treats `a-z' as `the letters a through
				* z', but treats `a-a' as `the letter a, the
				* character -, and the letter a'.
				*
				* For compatibility, the `-' is not considerd
				* to define a range if the character following
				* it is either a close bracket (required by ANSI)
				* or is not numerically greater than the character
				* we just stored in the table (c).
				*/
				n = *fmt;
				if (n == ']' || n < c) {
					c = '-';
					break;  /* resume the for(;;) */

					}
				fmt++;
				/* fill in the range */
				do {
					tab[++c] = v;

					} while (c < n);
					c = n;
					/*
					* Alas, the V7 Unix scanf also treats formats
					* such as [a-c-e] as `the letters a through e'.
					* This too is permitted by the standard....
					*/
					goto doswitch;
					break;

			case ']':               /* end of scanset */
				return (fmt);

			default:                /* just another character */
				c = n;
				break;

			}

		}
	/* NOTREACHED */
	}

int ICACHE_FLASH_ATTR vsscanf(const char *inp, char const *fmt0, va_list ap) {
	int inr;
	const u_char *fmt = (const u_char *)fmt0;
	int c;                  /* character from format, or conversion */
	size_t width;           /* field width, or 0 */
	char *p;                /* points into all kinds of strings */
	int n;                  /* handy integer */
	int flags;              /* flags as defined above */
	char *p0;               /* saves original value of p when necessary */
	int nassigned;          /* number of fields assigned */
	int nconversions;       /* number of conversions */
	int nread;              /* number of characters consumed from fp */
	int base;               /* base argument to strtoq/strtouq */
	ccfntype ccfn;          /* conversion function (strtoq/strtouq) */
	char ccltab[256];       /* character class table for %[...] */
	char buf[BUF];          /* buffer for numeric conversions */

	/* `basefix' is used to avoid `if' tests in the integer scanner */
	static short basefix[17] =
		{ 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

	inr = strlen(inp);

	nassigned = 0;
	nconversions = 0;
	nread = 0;
	base = 0;               /* XXX just to keep gcc happy */
	ccfn = NULL;            /* XXX just to keep gcc happy */
	for(;;) {
		c = *fmt++;
		if(c == 0)
			return (nassigned);
		if(isspace(c)) {
			while(inr > 0 && isspace(*inp))
				nread++, inr--, inp++;
			continue;

			}
		if(c != '%')
			goto literal;
		width = 0;
		flags = 0;
		/*
		* switch on the format.  continue if done;
		* break once format type is derived.
		*/
again:
		c = *fmt++;
		switch(c) {
			case '%':
literal :
				if(inr <= 0)
					goto input_failure;
				if(*inp != c)
					goto match_failure;
				inr--, inp++;
				nread++;
				continue;

			case '*':
				flags |= SUPPRESS;
				goto again;
			case 'l':
				flags |= LONG;
				goto again;
			case 'q':
				flags |= QUAD;
				goto again;
			case 'h':
				flags |= SHORT;
				goto again;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				width = width * 10 + c - '0';
				goto again;

				/*
				* Conversions.
				*
				*/
			case 'd':
				c = CT_INT;
				ccfn = (ccfntype)strtoq;
				base = 10;
				break;

			case 'i':
				c = CT_INT;
				ccfn = (ccfntype)strtoq;
				base = 0;
				break;

			case 'o':
				c = CT_INT;
				ccfn = strtouq;
				base = 8;
				break;

			case 'u':
				c = CT_INT;
				ccfn = strtouq;
				base = 10;
				break;

			case 'x':
				flags |= PFXOK; /* enable 0x prefixing */
				c = CT_INT;
				ccfn = strtouq;
				base = 16;
				break;

			case 's':
				c = CT_STRING;
				break;

			case '[':
				fmt = __sccl(ccltab, fmt);
				flags |= NOSKIP;
				c = CT_CCL;
				break;

			case 'c':
				flags |= NOSKIP;
				c = CT_CHAR;
				break;

			case 'p':       /* pointer format is like hex */
				flags |= POINTER | PFXOK;
				c = CT_INT;
				ccfn = strtouq;
				base = 16;
				break;

			case 'n':
				nconversions++;
				if(flags & SUPPRESS)    /* ??? */
					continue;
				if(flags & SHORT)
					* va_arg(ap, short *) = nread;
				else if(flags & LONG)
					* va_arg(ap, long *) = nread;
				else if(flags & QUAD)
					* va_arg(ap, quad_t *) = nread;
				else
					* va_arg(ap, int *) = nread;
				continue;

			}

		/*
		* We have a conversion that requires input.
		*/
		if(inr <= 0)
			goto input_failure;

		/*
		* Consume leading white space, except for formats
		* that suppress this.
		*/
		if((flags & NOSKIP) == 0) {
			while(isspace(*inp)) {
				nread++;
				if(--inr > 0)
					inp++;
				else
					goto input_failure;

				}
			/*
			* Note that there is at least one character in
			* the buffer, so conversions that do not set NOSKIP
			* can no longer result in an input failure.
			*/

			}

		/*
		* Do the conversion.
		*/
		switch(c) {

			case CT_CHAR:
				/* scan arbitrary characters (sets NOSKIP) */
				if(width == 0)
					width = 1;
				if(flags & SUPPRESS) {
					size_t sum = 0;
					for(;;) {
						if((n = inr) < width) {
							sum += n;
							width -= n;
							inp += n;
							if(sum == 0)
								goto input_failure;
							break;

							}
						else {
							sum += width;
							inr -= width;
							inp += width;
							break;

							}

						}
					nread += sum;

					}
				else {
					strlcpy(va_arg(ap, char *), inp, width+1);
					inr -= width;
					inp += width;
					nread += width;
					nassigned++;

					}
				nconversions++;
				break;

			case CT_CCL:
				/* scan a (nonempty) character class (sets NOSKIP) */
				if(width == 0)
					width = (size_t)~0;     /* `infinity' */
				/* take only those things in the class */
				if(flags & SUPPRESS) {
					n = 0;
					while(ccltab[(unsigned char)*inp]) {
						n++, inr--, inp++;
						if(--width == 0)
							break;
						if(inr <= 0) {
							if(n == 0)
								goto input_failure;
							break;

							}

						}
					if(n == 0)
						goto match_failure;

					}
				else {
					p0 = p = va_arg(ap, char *);
					while(ccltab[(unsigned char)*inp]) {
						inr--;
						* p++ = *inp++;
						if(--width == 0)
							break;
						if(inr <= 0) {
							if(p == p0)
								goto input_failure;
							break;

							}

						}
					n = p - p0;
					if(n == 0)
						goto match_failure;
					* p = 0;
					nassigned++;

					}
				nread += n;
				nconversions++;
				break;

			case CT_STRING:
				/* like CCL, but zero-length string OK, & no NOSKIP */
				if(width == 0)
					width = (size_t)~0;
				if(flags & SUPPRESS) {
					n = 0;
					while(!isspace(*inp)) {
						n++, inr--, inp++;
						if(--width == 0)
							break;
						if(inr <= 0)
							break;

						}
					nread += n;

					}
				else {
					p0 = p = va_arg(ap, char *);
					while(!isspace(*inp)) {
						inr--;
						* p++ = *inp++;
						if(--width == 0)
							break;
						if(inr <= 0)
							break;

						}
					* p = 0;
					nread += p - p0;
					nassigned++;

					}
				nconversions++;
				continue;

			case CT_INT:
				/* scan an integer as if by strtoq/strtouq */
#ifdef hardway
				if(width == 0 || width > sizeof(buf) - 1)
					width = sizeof(buf) - 1;
#else
				/* size_t is unsigned, hence this optimisation */
				if(--width > sizeof(buf) - 2)
					width = sizeof(buf) - 2;
				width++;
#endif
				flags |= SIGNOK | NDIGITS | NZDIGITS;
				for(p = buf; width; width--) {
					c = *inp;
					/*
					* Switch on the character; `goto ok'
					* if we accept it as a part of number.
					*/
					switch(c) {

							/*
							* The digit 0 is always legal, but is
							* special.  For %i conversions, if no
							* digits (zero or nonzero) have been
							* scanned (only signs), we will have
							* base==0.  In that case, we should set
							* it to 8 and enable 0x prefixing.
							* Also, if we have not scanned zero digits
							* before this, do not turn off prefixing
							* (someone else will turn it off if we
							* have scanned any nonzero digits).
							*/
						case '0':
							if(base == 0) {
								base = 8;
								flags |= PFXOK;

								}
							if(flags & NZDIGITS)
								flags &= ~(SIGNOK | NZDIGITS | NDIGITS);
							else
								flags &= ~(SIGNOK | PFXOK | NDIGITS);
							goto ok;

							/* 1 through 7 always legal */
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
							base = basefix[base];
							flags &= ~(SIGNOK | PFXOK | NDIGITS);
							goto ok;

							/* digits 8 and 9 ok iff decimal or hex */
						case '8':
						case '9':
							base = basefix[base];
							if(base <= 8)
								break;  /* not legal here */
							flags &= ~(SIGNOK | PFXOK | NDIGITS);
							goto ok;

							/* letters ok iff hex */
						case 'A':
						case 'B':
						case 'C':
						case 'D':
						case 'E':
						case 'F':
						case 'a':
						case 'b':
						case 'c':
						case 'd':
						case 'e':
						case 'f':
							/* no need to fix base here */
							if(base <= 10)
								break;  /* not legal here */
							flags &= ~(SIGNOK | PFXOK | NDIGITS);
							goto ok;

							/* sign ok only as first character */
						case '+':
						case '-':
							if(flags & SIGNOK) {
								flags &= ~SIGNOK;
								goto ok;

								}
							break;

							/* x ok iff flag still set & 2nd char */
						case 'x':
						case 'X':
							if(flags & PFXOK && p == buf + 1) {
								base = 16;      /* if %i */
								flags &= ~PFXOK;
								goto ok;

								}
							break;

						}

					/*
					* If we got here, c is not a legal character
					* for a number.  Stop accumulating digits.
					*/
					break;
ok:
					/*
					* c is legal: store it and look at the next.
					*/
					* p++ = c;
					if(--inr > 0)
						inp++;
					else
						break;          /* end of input */

					}
				/*
				* If we had only a sign, it is no good; push
				* back the sign.  If the number ends in `x',
				* it was [sign] '0' 'x', so push back the x
				* and treat it as [sign] '0'.
				*/
				if(flags & NDIGITS) {
					if(p > buf) {
						inp--;
						inr++;

						}
					goto match_failure;

					}
				c = ((u_char *)p)[-1];
				if(c == 'x' || c == 'X') {
					--p;
					inp--;
					inr++;

					}
				if((flags & SUPPRESS) == 0) {
					u_quad_t res;

					* p = 0;
					res = (*ccfn)(buf, (char **)NULL, base);
					if(flags & POINTER)
						* va_arg(ap, void **) =
							(void *)(unsigned)res;
					else if(flags & SHORT)
						* va_arg(ap, short *) = res;
					else if(flags & LONG)
						* va_arg(ap, long *) = res;
					else if(flags & QUAD)
						* va_arg(ap, quad_t *) = res;
					else
						* va_arg(ap, int *) = res;
					nassigned++;

					}
				nread += p - buf;
				nconversions++;
				break;


			}

		}
input_failure:
	return (nconversions != 0 ? nassigned : -1);
match_failure:
	return (nassigned);
	}

int ICACHE_FLASH_ATTR sscanf(const char *ibuf, const char *fmt, ...) {
		va_list ap;
		int ret;

		va_start(ap, fmt);
		ret = vsscanf(ibuf, fmt, ap);
		va_end(ap);
		return(ret);
		}



