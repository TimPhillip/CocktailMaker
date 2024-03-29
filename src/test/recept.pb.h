/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.9.1 at Thu May 30 21:43:52 2019. */

#ifndef PB_RECEPT_PB_H_INCLUDED
#define PB_RECEPT_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _Ingredient {
    char name[50];
    float amount;
/* @@protoc_insertion_point(struct:Ingredient) */
} Ingredient;

typedef struct _Liquid {
    char name[50];
    int32_t pump_id;
/* @@protoc_insertion_point(struct:Liquid) */
} Liquid;

typedef struct _Recept {
    char name[50];
    pb_callback_t ingedients;
/* @@protoc_insertion_point(struct:Recept) */
} Recept;

typedef struct _CocktailSetup {
    pb_size_t liquids_count;
    Liquid liquids[4];
/* @@protoc_insertion_point(struct:CocktailSetup) */
} CocktailSetup;

/* Default values for struct fields */

/* Initializer values for message structs */
#define Ingredient_init_default                  {"", 0}
#define Recept_init_default                      {"", {{NULL}, NULL}}
#define CocktailSetup_init_default               {0, {Liquid_init_default, Liquid_init_default, Liquid_init_default, Liquid_init_default}}
#define Liquid_init_default                      {"", 0}
#define Ingredient_init_zero                     {"", 0}
#define Recept_init_zero                         {"", {{NULL}, NULL}}
#define CocktailSetup_init_zero                  {0, {Liquid_init_zero, Liquid_init_zero, Liquid_init_zero, Liquid_init_zero}}
#define Liquid_init_zero                         {"", 0}

/* Field tags (for use in manual encoding/decoding) */
#define Ingredient_name_tag                      1
#define Ingredient_amount_tag                    2
#define Liquid_name_tag                          1
#define Liquid_pump_id_tag                       2
#define Recept_name_tag                          1
#define Recept_ingedients_tag                    2
#define CocktailSetup_liquids_tag                1

/* Struct field encoding specification for nanopb */
extern const pb_field_t Ingredient_fields[3];
extern const pb_field_t Recept_fields[3];
extern const pb_field_t CocktailSetup_fields[2];
extern const pb_field_t Liquid_fields[3];

/* Maximum encoded size of messages (where known) */
#define Ingredient_size                          57
/* Recept_size depends on runtime parameters */
#define CocktailSetup_size                       260
#define Liquid_size                              63

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define RECEPT_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif
