#include <stdint.h>
#include <ruby.h>

const double TWO_POW_MINUS_53 = 1.0L/9007199254740992.0L;

uint64_t
xoroshiro128plus(uint64_t s[2])
{
    uint64_t s0 = s[0];
    uint64_t s1 = s[1];
    uint64_t result = s0 + s1;
    s1 ^= s0;
    s[0] = ((s0 << 55) | (s0 >> 9)) ^ s1 ^ (s1 << 14);
    s[1] = (s1 << 36) | (s1 >> 28);
    return result;
}

double
xoroshiro128plus_double(uint64_t s[2])
{
    uint64_t rs = xoroshiro128plus(s);
    /* In C, doubles have 53 significant bits (see, e.g.,
     * https://en.wikipedia.org/wiki/IEEE_754). So, we are going to drop 13
     * bits from rs and then divide by 1/2**53 to normalize the output within
     * the (0.0L, 1.0L) interval. */
    return (rs >> 13) * TWO_POW_MINUS_53;
}

struct random_state {
  uint64_t s[2];
};

static const rb_data_type_t random_type = {
    .wrap_struct_name = "xoroshiro128plus",
    .function = {
        .dfree = RUBY_DEFAULT_FREE,
    },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE fast_cext_random_alloc(VALUE self)
{
    struct random_state* data;
    return TypedData_Make_Struct(self, struct random_state, &random_type, data);
}

VALUE
fast_cext_random_init(VALUE self, VALUE seed1, VALUE seed2)
{
    struct random_state* data;
    TypedData_Get_Struct(self, struct random_state, &random_type, data);
    data->s[0] = NUM2ULONG(seed1);
    data->s[1] = NUM2ULONG(seed2);
    return self;
}

VALUE
fast_cext_random_rand(VALUE self)
{
    struct random_state* data;
    double result;
    TypedData_Get_Struct(self, struct random_state, &random_type, data);
    result = xoroshiro128plus_double(data->s);
    return DBL2NUM(result);
}

void Init_random_cext(void) {
    VALUE mod = rb_define_module("FastCExtRandom");
    VALUE random = rb_define_class_under(mod, "Random", rb_cObject);
    rb_define_alloc_func(random, fast_cext_random_alloc);
    rb_define_private_method(random, "init", fast_cext_random_init, 2);
    rb_define_method(random, "rand", fast_cext_random_rand, 0);
}
