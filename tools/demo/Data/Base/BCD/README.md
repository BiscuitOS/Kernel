BCD AND PACKED BCD INTEGERS
-----------------------------------------

Binary-coded decimal integers (BCD integers) are unisgned 4-bit integers 
with valid values ranging from 0 to 9. IA-32 architecture defines operations
on BCD integers located in one or more general-purpose register or in one
or more X87 FPU register.

```
BCD integers

  7                      4 3                              0
  +-----------------------+-------------------------------+
  |          X            |         BCD                   |
  +-------------------------------------------------------+                        

Packed BCD Integers

  7                      4 3                              0
  +-----------------------+-------------------------------+
  |         BCD           |         BCD                   |
  +-------------------------------------------------------+                        
```

In computing and electronic system, binary-code decimal (BCD) is a class
of binary encodings of decimal numbers where each decimal digit is 
represented by a fixed number of bits, usually four or eight. Special bit
patterns are sometimes used for a sign or for other indications (e.g.
error or overflow).

In byte-oriented system (i.e. most modern computers), the term `unpacked BCD`
usually implies a full byte for each digit (often including a sign), whereas 
`packed BCD` typically encodes two decimal digits within a single byte by
taking advantage of the fact that four bits are enough to represent the range
0 to 9. The precise 4-bit encoding may vary however, for technical reasons,
see Excess-3 for instance. The ten states representing a BCD decimal digit
are sometimes called terades (for the nibble typically needed to hold them
also know as tetrad) with those don't care-states unused named pseudo-tetrad
or pseudo-decimal digit.

BCD's main virtue is its more accurate representation and rounding of decimal
quantities as well as an ease of conversion into human-readable 
representations, in comparison to binary positional systems. BCD's principal
darwbacks are a samll increase in the complexity of the circuits needed to 
implement basic arithmetics and a slightly less dense storage.

## Basics

`BCD` takes advantage of the fact that any one decimal numeral can be 
represented by a four bit pattern. The most obvious way of encoding digits is
"natural BCD" (NBCD), where each decimal digit is represented by its
corresponding four-bit binary value, as show in the following table.

| Decimal digit |   8421 BCD   |   2421 BCD   |   Excess-3   |
| ------------- | ------------ | ------------ | ------------ |
| 0             | 0000         | 0000         | 0011         |
| 1             | 0001         | 0001         | 0100         |
| 2             | 0010         | 0010         | 0101         |
| 3             | 0011         | 0011         | 0110         |
| 4             | 0100         | 0100         | 0111         |
| 5             | 0101         | 1011         | 1000         |
| 6             | 0110         | 1100         | 1001         |
| 7             | 0111         | 1101         | 1010         |
| 8             | 1000         | 1110         | 1011         |
| 9             | 1001         | 1111         | 1100         |

Other encodings are also used, including so-called `4221` and `7421` -- named
after the weighting used for the bits -- and `Excess-3`. For example, the
`BCD` digit 6, '0110'b in `8421` notation, is `1100`b in 4221 (two encodings
are possible). `0110`b in 7421, and `1001`b in excess-3.

As most computers deal with data in 8-bit bytes, it is possible to use one
of the following methods to encode a BCD number:

* Unpacked

  Each numeral is encoded into one byte, with four bits representing the 
  and the remaining bits having no significance.

* Packed

  Two numerals are encoded into a single byte, with one numeral in the least
  significant nibble (bits 0 through 3) and the other numeral in the most
  significant nibble (bits 4 through 7).

As an example, encoding the decimal number 91 using unpacked BCD result in
following binary pattern of two bytes:

```
  Decimal:          9         1
  Binary :  0000 1001 0000 0001  
```

In packed BCD, the same number would fit into a single byte:

```
  Decimal:     9    1
  Binary :  1001 0001
```

## Addition with BCD

It is possible to perform addition in BCD by first adding in binary, and
then converting to BCD afterwards. Conversion of the simple sum of two 
digits can be done by adding 6 (that is, 16 - 10) when the five-bit result
of adding a pair of digits has a value greater than 9. For example:

```
  1001 + 1000 = 10001
     9 +    8 = 17
```

Note that `10001` is the binary, not decimal, representation of the desired
result. Also note that it cannot fit in a 4-bit number. In BCD as in decimal,
there cannot exist a value greater than 9 (1001) per digit. To correct this,
6 (0110) is added to that sum and then the result is treated as two nibbles:

```
  10001 + 0110 = 00010111 => 0001 0111
     17 +    6 =       23       1    7
```

The two nibbles of the result, `0001` and `0111`, correspond to the digit `1`
and `7`. This yields `17` in BCD, which is the correct result.

This technique can be extended to adding multiple digits by adding in groups
from right to left, propagating the second digit as a carry, always comparing
the 5-bit result of each digit-pair sum to 9. Some CPUs provide a half-carry
flag to facilitate BCD arithmetic adjustments following binary addition and
subtraction operations.

## link

  [Intel Architectures Software Developer's Manual: Combined Volumes: 1 -- Chapter 4 Data Types: 4.7 BCD and Packed BCD Integers ](https://software.intel.com/en-us/articles/intel-sdm)

  [BCD on Wikipedia](https://en.wikipedia.org/wiki/Binary-coded_decimal)
