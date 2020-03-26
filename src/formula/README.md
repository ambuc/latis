# ABNF Grammar for formulas.

Reading [A Guide To
Parsing](https://tomassetti.me/guide-parsing-algorithms-terminology/#definition)
for help on this.

Specified in 
[https://en.wikipedia.org/wiki/Augmented_Backus%E2%80%93Naur_form](ABNF).

Datetime parsing stolen from 
[https://tools.ietf.org/html/rfc3339#section-5.6](RFC3339).

    
## Workflow

```

           lexer           parser              evaluator
plaintext <=====> [token] <======> Expression <=========> Amount

plaintext  : =POW(A2,4)
tokens     : equals prefix_op_2 '(' location_value ',' double_value rparen
expression : op_binary { 
               operation: Pow
               lhs: { lookup: { row: 2 col: 0 } }
               rhs: { value: 4 }
             }
amount     : ?

```


## Grammar

I think this is a context-free grammar, since it is recursive (EXPR can contain
EXPR).

Per ABNF:
*   `X / Y` means either `X` or `Y`.
*   `%d` means decimal numbers.
*   `0-9` means a range of values.
*   `<a>*<b>element` means `element` between `a` and `b` times.  `a` has a 
    default of 0, and `b` has a default of infinity.  So `1*e` means `e` one or
    more times.
*   `<a>element` means `element` exactly `n` times.
*   `[e]` means that the element `e` is optional.

The following grammar is left-recursive, since `EXPR ?= EXPR "?" EXPR ":" EXPR`.

This grammar respects order of precedence in PEMDAS.


    DIGIT            = %d0-9
    
    DATE_FULLYEAR    = 4DIGIT
    DATE_MONTH       = 2DIGIT  ; 01-12
    DATE_MDAY        = 2DIGIT  ; 01-28, 01-29, 01-30, 01-31 based on month/year
    TIME_HOUR        = 2DIGIT  ; 00-23
    TIME_MINUTE      = 2DIGIT  ; 00-59
    TIME_SECOND      = 2DIGIT  ; 00-58, 00-59, 00-60 based on leap second rules
    TIME_SECFRAC     = "." 1*DIGIT
    TIME_NUMOFFSET   = ("+" / "-") TIME_HOUR ":" TIME_MINUTE
    TIME_OFFSET      = "Z" / time_numoffset
    DATE_TIME        = DATE_FULLYEAR "-" DATE_MONTH "-" DATE_MDAY "T"
                       TIME_HOUR ":" TIME_MINUTE ":" TIME_SECOND [TIME_SECFRAC] 
                       TIME_OFFSET
    
    UPPERCASE        = %c"A"-"Z"
    ALPHANUMERIC     = 1*(UPPERCASE / DIGIT)
    
    INT_NUMERIC      = 1*DIGIT
    DOUBLE_NUMERIC   = *DIGIT "." *DIGIT
    NUM_VAL          = INT_NUMERIC | DOUBLE_NUMERIC
    
    CURRENCY_ENUM    = "USD" / "SEK" / ...
    CURRENCY_VAL     = NUM_VAL CURRENCY_ENUM
    
    STR_VAL          = "\"" *e "\""                      ; anything that's not 
                                                         ; a quote can go here.
    
    ROW_INDICATOR    = 1*DIGIT
    COL_INDICATOR    = 1*UPPERCASE
    
    LOCATION_VAL     = COL_INDICATOR ROW_INDICATOR       ; A1
    RANGE_VAL        = COL_INDICATOR ":" COL_INDICATOR   ; A:B
                     / LOCATION_VAL ":" ROW_INDICATOR    ; A1:3
                     / LOCATION_VAL ":" LOCATION_VAL     ; A1:B3

    VAL              = CURRENCY_VAL
                     / DATE_TIME
                     / NUM_VAL
                     / STR_VAL
                     / LOCATION_VAL
                     / RANGE_VAL
    
    FN               = 1*(ALPHANUMERIC / "_")

    EXPR             = VAL                               ; 4
                     / "(" EXPR ")"                      ; (5)
                     / "(" EXPR FN EXPR ")"              ; (1+1)
                     / FN "(" EXPR *( "," EXPR ) ")"     ; POW(2,4,6)
                     / EXPR "?" EXPR ":" EXPR            ; cond ? val1 : val2
    
    GRAMMAR          = "=" EXPR
