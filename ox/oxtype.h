//
//  ox/oxtype.h
//
//  Created by Michael Henderson on 10/3/14.
//  Copyright (c) 2014 Michael D Henderson. All rights reserved.
//

#ifndef __ox__oxtype__
#define __ox__oxtype__

// atom   - type atom
// list   - type list
// symbol - type list, flagged as symbol
//        - attributes stored in associative list
//             (('name' 'pi') ('value' 3.14159))
//
// boolean
//   ox does not have a boolean atom
//   nill is always false
//   everything else is always true
//
// lists and cons cells
//
//   ox does not support the traditional 'cons' operator
//
//   in lists, 'car' points to a data cell
//             'cdr' points to  'nill' to end the list
//                or points to the next cell in the list
//       (car -> any valid data cell (including nill)
//        cdr -> next cell in list or nill to terminate
//       )
//                cdr -> (car -> any valid data cell
//                        cdr -> nill (terminates list)
//                       )
//               )
//       )
//
// examples
//   1
//   atom: 1
//
//   ()
//   atom: nill
//
//   ()
//   car|cdr -> nill
//    |
//    v
//   nill
//
//   (1 2 3)
//   car|cdr -> car|cdr -> car|cdr -> nill
//    |          |          |
//    v          v          v
//    1          2          3
//
//   (1 (2) 3)
//   car|cdr -> car|cdr -> car|cdr -> nill
//    |          |          |
//    v          |          v
//    1          |          3
//               v
//              car|cdr -> nill
//               |
//               v
//               2
//
//   (key value)
//   car|cdr -> car|cdr -> nill
//    |          |
//    v          v
//   key       value
//
//   (1 (key value) (key value) 4)
//   car|cdr -> car|cdr -> car|cdr -> car|cdr -> nill
//    |          |          |          |
//    v          |          |          v
//    1          |          |          4
//               |          v
//               |         car|cdr -> car|cdr -> nill
//               |          |          |
//               |          v          v
//               |         key       value
//               |
//              car|cdr -> car|cdr -> nill
//               |          |
//               v          v
//              key       value
//
//   (nill)
//   car|cdr -> car|cdr -> nill
//    |          |
//    v          v
//   nill       nill
//
//   (nill (nill) nill)
//   car|cdr -> car|cdr -> car|cdr -> nill
//    |          |          |
//    v          |          v
//   nill        |          nill
//               v
//              car|cdr -> nill
//               |
//               v
//              nill
//

enum atomType {eAtomFunc, eAtomInteger, eAtomReal, eAtomText, eAtomTime};

typedef struct oxatom {
    enum atomType    kind;      // kind for atoms
    struct oxcell *(*func)(struct oxcell *, struct oxcell *);
    long             integer;   // value for integer
    double           real;      // value for real
    char            *text;      // text for strings
    unsigned long    timestamp; // time values
} oxatom;

typedef struct oxcell {
    enum { octAtom, octCons } kind;
    
    oxatom   *atom;
    int       isSymbol;
    
    struct {
        struct oxcell *car;
        struct oxcell *cdr;
    } cons;
} oxcell;

typedef struct oxtoken {
    union {
        long   integer;
        char  *name;
        double real;
        char  *text;
    } value;
    enum {oxTokEOF, oxTokOpenParen, oxTokCloseParen, oxTokInteger, oxTokName, oxTokReal, oxTokText} kind;
    int line;
    unsigned char data[1];
} oxtoken;


#endif /* defined(__ox__oxtype__) */
