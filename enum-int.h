#undef ENUM
// 32 bits:
// bit:0 - bit:15 type bits
// bit:16 - bit:31 ordinal value  (use high bits so at least '<' and '>' work
//                                 without regard to type bits when comparing
//                                 two enumerated values)
#define ENUM(name, ordinal_value, type_bits)                            \
  name = ((uint32_t) ((ordinal_value) << 16) | ((uint32_t) type_bits))

#define GET_TYPE_BITS(e) ((e) & 0xffff)

#define GET_ORDINAL(e) ((int) ((e) >> 16))

#define HAS_ANY_TYPE(e, type_bits) \
  ((uint32_t) GET_TYPE_BITS(e) & ((uint32_t) (type_bits)))

#define HAS_ALL_TYPES(n, type_bits) \
  (((uint32_t) GET_TYPE_BITS(n)) == ((uint32_t) (type_bits)))
