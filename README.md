# static_reflection

static_reflection in compiler time

#example

```

struct Test
{
    int32_t a;
    float b;
    std::string c;
};
DEFINE_META(Test,
            DEFINE_MEMBER(
              META_MEMBER(a),
              META_MEMBER(b),
              META_MEMBER(c));

Test test;
json_to_struct<T>(json, test);


```              
