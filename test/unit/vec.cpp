#include <phypp.hpp>

#define check_base_(cond, msg, line) do { \
    print("vec.cpp:"+strn(line)); \
    if (!(cond)) print(msg); \
    assert(cond); \
} while (false)

#define check_base(cond, msg) check_base_(cond, msg, __LINE__)

#define check(t, s) \
    check_base(t == s, "  failed: "+std::string(#t)+" = "+strn(t)+" != "+strn(s))

#define check_float_eps(t, s, e) \
    check_base(fabs(t - s) <= e || (nan(t) && nan(s)), \
        "  failed: "+std::string(#t)+" = "+strn(t)+" != "+strn(s))

#define check_float(t, s) \
    check_float_eps(t, s, std::numeric_limits<decltype(fabs(t - s))>::epsilon())

template<typename T>
void assert_is_vec() {
    static_assert(is_vec<T>::value, "failed is_vec");
}

template<typename T>
void assert_not_is_vec() {
    static_assert(!is_vec<T>::value, "failed !is_vec");
}

void test_is_vec() {
    print("test_is_vec...");

    // Metaprogramming
    vec1u r;

    assert_is_vec<vec1i>();
    assert_is_vec<vec1u>();
    assert_is_vec<vec1s>();
    assert_is_vec<vec1f>();
    assert_is_vec<vec1b>();
    assert_is_vec<vec1d>();
    assert_is_vec<vec2s>();
    assert_is_vec<vec2f>();
    assert_is_vec<vec2d>();
    assert_is_vec<vec2s>();
    assert_is_vec<vec2b>();
    assert_is_vec<decltype(vec1f()(_))>();
    assert_is_vec<decltype(vec1f()(r))>();
    assert_is_vec<decltype(vec1f()[_])>();
    assert_is_vec<decltype(vec1f()[r])>();
    assert_is_vec<decltype(vec2b()[_])>();
    assert_is_vec<decltype(vec2b()[r])>();
    assert_is_vec<decltype(vec2b()(_,0))>();
    assert_is_vec<decltype(vec2b()(r,0))>();
    assert_is_vec<decltype(vec2b()(0,_))>();
    assert_is_vec<decltype(vec2b()(0,r))>();
    assert_is_vec<decltype(vec3s()(_,_,0))>();
    assert_is_vec<decltype(vec3s()(r,_,0))>();
    assert_is_vec<decltype(vec3s()(_,r,0))>();
    assert_is_vec<decltype(vec3s()(r,r,0))>();
    assert_is_vec<decltype(vec3s()(_,0,_))>();
    assert_is_vec<decltype(vec3s()(r,0,_))>();
    assert_is_vec<decltype(vec3s()(_,0,r))>();
    assert_is_vec<decltype(vec3s()(r,0,r))>();
    assert_is_vec<decltype(vec3s()(0,_,_))>();
    assert_is_vec<decltype(vec3s()(0,r,_))>();
    assert_is_vec<decltype(vec3s()(0,_,r))>();
    assert_is_vec<decltype(vec3s()(0,r,r))>();

    assert_not_is_vec<float>();
    assert_not_is_vec<std::vector<int>>();
    assert_not_is_vec<std::string>();
    assert_not_is_vec<decltype(vec1s()(0))>();
    assert_not_is_vec<decltype(vec1b()(0))>();
    assert_not_is_vec<decltype(vec2i()[0])>();
    assert_not_is_vec<decltype(vec2i()(0,0))>();
    assert_not_is_vec<decltype(vec3u()(0,0,0))>();

    print("> passed");
}

template<typename T, std::size_t D>
void assert_vec_dim() {
    static_assert(vec_dim<T>::value == D, "failed vec_dim");
}

void test_vec_dim() {
    print("test_vec_dim...");

    // Metaprogramming
    vec1u r;

    assert_vec_dim<float,0>();
    assert_vec_dim<std::string,0>();
    assert_vec_dim<std::vector<int>,0>();
    assert_vec_dim<vec1d,1>();
    assert_vec_dim<vec2s,2>();
    assert_vec_dim<vec3f,3>();
    assert_vec_dim<decltype(vec1f()(_)),1>();
    assert_vec_dim<decltype(vec1f()(r)),1>();
    assert_vec_dim<decltype(vec1f()[_]),1>();
    assert_vec_dim<decltype(vec1f()[r]),1>();
    assert_vec_dim<decltype(vec2d()[_]),1>();
    assert_vec_dim<decltype(vec2d()[r]),1>();
    assert_vec_dim<decltype(vec3s()[_]),1>();
    assert_vec_dim<decltype(vec3s()[r]),1>();
    assert_vec_dim<decltype(vec2f()(_,0)),1>();
    assert_vec_dim<decltype(vec2f()(r,0)),1>();
    assert_vec_dim<decltype(vec2f()(0,_)),1>();
    assert_vec_dim<decltype(vec2f()(0,r)),1>();
    assert_vec_dim<decltype(vec3b()(_,_,0)),2>();
    assert_vec_dim<decltype(vec3b()(r,_,0)),2>();
    assert_vec_dim<decltype(vec3b()(_,r,0)),2>();
    assert_vec_dim<decltype(vec3b()(r,r,0)),2>();
    assert_vec_dim<decltype(vec3b()(_,0,_)),2>();
    assert_vec_dim<decltype(vec3b()(r,0,_)),2>();
    assert_vec_dim<decltype(vec3b()(_,0,r)),2>();
    assert_vec_dim<decltype(vec3b()(r,0,r)),2>();
    assert_vec_dim<decltype(vec3b()(0,_,_)),2>();
    assert_vec_dim<decltype(vec3b()(0,r,_)),2>();
    assert_vec_dim<decltype(vec3b()(0,_,r)),2>();
    assert_vec_dim<decltype(vec3b()(0,r,r)),2>();

    print("> passed");
}

void test_vec_constructor() {
    print("test_vec_constructor...");

    {
        // Default constructor
        vec1d v01;
        vec2s v02;
        vec3i v03;

        check(v01.dims[0], 0);
        check(v02.dims[0], 0);
        check(v02.dims[1], 0);
        check(v03.dims[0], 0);
        check(v03.dims[1], 0);
        check(v03.dims[2], 0);

        check(v01.size(), 0);
        check(v02.size(), 0);
        check(v03.size(), 0);

        check(v01.empty(), true);
        check(v02.empty(), true);
        check(v03.empty(), true);
    }

    {
        // Size constructor
        const uint_t d1 = 10;
        const uint_t d21 = 10;
        const uint_t d22 = 20;
        const uint_t d31 = 5;
        const uint_t d32 = 7;
        const uint_t d33 = 15;
        const uint_t dr = 3;

        vec1u r(dr);
        vec1d v1(d1);
        vec2s v2(d21,d22);
        vec3b v3(d31,d32,d33);

        check(v1.dims[0], d1);
        check(v2.dims[0], d21);
        check(v2.dims[1], d22);
        check(v3.dims[0], d31);
        check(v3.dims[1], d32);
        check(v3.dims[2], d33);
        check(v1[_].dims[0], d1);
        check(v1[r].dims[0], dr);
        check(v2[_].dims[0], d21*d22);
        check(v2[r].dims[0], dr);
        check(v3[_].dims[0], d31*d32*d33);
        check(v3[r].dims[0], dr);
        check(v2(_,0).dims[0], d21);
        check(v2(r,0).dims[0], dr);
        check(v2(0,_).dims[0], d22);
        check(v2(0,r).dims[0], dr);
        check(v3(_,_,0).dims[0], d31);
        check(v3(r,_,0).dims[0], dr);
        check(v3(_,r,0).dims[0], d31);
        check(v3(r,r,0).dims[0], dr);
        check(v3(_,_,0).dims[1], d32);
        check(v3(r,_,0).dims[1], d32);
        check(v3(_,r,0).dims[1], dr);
        check(v3(r,r,0).dims[1], dr);
        check(v3(_,0,_).dims[0], d31);
        check(v3(r,0,_).dims[0], dr);
        check(v3(_,0,r).dims[0], d31);
        check(v3(r,0,r).dims[0], dr);
        check(v3(_,0,_).dims[1], d33);
        check(v3(r,0,_).dims[1], d33);
        check(v3(_,0,r).dims[1], dr);
        check(v3(r,0,r).dims[1], dr);
        check(v3(0,_,_).dims[0], d32);
        check(v3(0,r,_).dims[0], dr);
        check(v3(0,_,r).dims[0], d32);
        check(v3(0,r,r).dims[0], dr);
        check(v3(0,_,_).dims[1], d33);
        check(v3(0,r,_).dims[1], d33);
        check(v3(0,_,r).dims[1], dr);
        check(v3(0,r,r).dims[1], dr);

        check(v1.size(), d1);
        check(v2.size(), d21*d22);
        check(v3.size(), d31*d32*d33);
        check(v1[_].size(), d1);
        check(v1[r].size(), dr);
        check(v2[_].size(), d21*d22);
        check(v2[r].size(), dr);
        check(v3[_].size(), d31*d32*d33);
        check(v3[r].size(), dr);
        check(v2(_,0).size(), d21);
        check(v2(r,0).size(), dr);
        check(v2(0,_).size(), d22);
        check(v2(0,r).size(), dr);
        check(v3(_,_,0).size(), d31*d32);
        check(v3(r,_,0).size(), dr*d32);
        check(v3(_,r,0).size(), d31*dr);
        check(v3(r,r,0).size(), dr*dr);
        check(v3(_,0,_).size(), d31*d33);
        check(v3(r,0,_).size(), dr*d33);
        check(v3(_,0,r).size(), d31*dr);
        check(v3(r,0,r).size(), dr*dr);
        check(v3(0,_,_).size(), d32*d33);
        check(v3(0,r,_).size(), dr*d33);
        check(v3(0,_,r).size(), d32*dr);
        check(v3(0,r,r).size(), dr*dr);

        check(v1.empty(), false);
        check(v2.empty(), false);
        check(v3.empty(), false);
        check(v1[_].empty(), false);
        check(v1[r].empty(), false);
        check(v2[_].empty(), false);
        check(v2[r].empty(), false);
        check(v3[_].empty(), false);
        check(v3[r].empty(), false);
        check(v2(_,0).empty(), false);
        check(v2(r,0).empty(), false);
        check(v2(0,_).empty(), false);
        check(v2(0,r).empty(), false);
        check(v3(_,_,0).empty(), false);
        check(v3(r,_,0).empty(), false);
        check(v3(_,r,0).empty(), false);
        check(v3(r,r,0).empty(), false);
        check(v3(_,0,_).empty(), false);
        check(v3(r,0,_).empty(), false);
        check(v3(_,0,r).empty(), false);
        check(v3(r,0,r).empty(), false);
        check(v3(0,_,_).empty(), false);
        check(v3(0,r,_).empty(), false);
        check(v3(0,_,r).empty(), false);
        check(v3(0,r,r).empty(), false);


        bool res = true;
        for (uint_t& v : r) {
            res = res && v == 0u;
        }
        check(res, true);
        res = true;
        for (double& v : v1) {
            res = res && v == 0.0;
        }
        check(res, true);
        res = true;
        for (std::string& v : v2) {
            res = res && v == "";
        }
        check(res, true);
        res = true;
        for (bool& v : v3) {
            res = res && v == false;
        }
        check(res, true);
    }

    {
        // Initializer list constructor
        vec1d v1({0.0, 1.0, 2.0, 5.0, 50.0});
        check(v1.dims[0], 5);
        check(v1.size(), 5);
        check(v1.empty(), false);
        check_float(v1[0], 0.0);
        check_float(v1[1], 1.0);
        check_float(v1[2], 2.0);
        check_float(v1[3], 5.0);
        check_float(v1[4], 50.0);

        vec2b v2({{true, false}, {false, true}, {true, true}});
        check(v2.dims[0], 3);
        check(v2.dims[1], 2);
        check(v2.size(), 6);
        check(v2.empty(), false);
        check_float(v2[0], true);
        check_float(v2(0,0), true);
        check_float(v2[1], false);
        check_float(v2(0,1), false);
        check_float(v2[2], false);
        check_float(v2(1,0), false);
        check_float(v2[3], true);
        check_float(v2(1,1), true);
        check_float(v2[4], true);
        check_float(v2(2,0), true);
        check_float(v2[5], true);
        check_float(v2(2,1), true);

        vec3s v3({});
        check(v3.dims[0], 0);
        check(v3.dims[1], 0);
        check(v3.dims[2], 0);
        check(v3.size(), 0);
        check(v3.empty(), true);
    }

    {
        // Move constructor
        const uint_t d1 = 10;
        const uint_t d21 = 10;
        const uint_t d22 = 20;
        const uint_t d31 = 5;
        const uint_t d32 = 7;
        const uint_t d33 = 15;

        vec1d v1(d1);
        vec2s v2(d21,d22);
        vec3b v3(d31,d32,d33);

        vec1d nv1(std::move(v1));
        vec2s nv2(std::move(v2));
        vec3b nv3(std::move(v3));

        check(nv1.dims[0], d1);
        check(v1.dims[0], 0);
        check(nv2.dims[0], d21);
        check(nv2.dims[1], d22);
        check(v2.dims[0], 0);
        check(v2.dims[1], 0);
        check(nv3.dims[0], d31);
        check(nv3.dims[1], d32);
        check(nv3.dims[2], d33);
        check(v3.dims[0], 0);
        check(v3.dims[1], 0);
        check(v3.dims[2], 0);

        check(nv1.size(), d1);
        check(v1.size(), 0);
        check(nv2.size(), d21*d22);
        check(v2.size(), 0);
        check(nv3.size(), d31*d32*d33);
        check(v3.size(), 0);

        bool res = true;
        for (double& v : nv1) {
            res = res && v == 0.0;
        }
        check(res, true);
        res = true;
        for (std::string& v : nv2) {
            res = res && v == "";
        }
        check(res, true);
        res = true;
        for (bool& v : nv3) {
            res = res && v == false;
        }
        check(res, true);

        vec1d w1({0.0, 5.0, -2.0, dnan});
        vec2s w2({{"foo", "bar", "goo"}, {"blop", "plop", "flop"}});
        vec3b w3({{{true}, {false}}, {{false}, {true}}, {{true}, {true}}});

        vec1d nw1(std::move(w1));
        vec2s nw2(std::move(w2));
        vec3d nw3(std::move(w3));

        check_float(nw1[0], 0.0);
        check_float(nw1[1], 5.0);
        check_float(nw1[2], -2.0);
        check_float(nw1[3], dnan);
        check(nw2[0], "foo");
        check(nw2[1], "bar");
        check(nw2[2], "goo");
        check(nw2[3], "blop");
        check(nw2[4], "plop");
        check(nw2[5], "flop");
        check(nw3[0], true);
        check(nw3[1], false);
        check(nw3[2], false);
        check(nw3[3], true);
        check(nw3[4], true);
        check(nw3[5], true);
    }

    {
        // Copy constructor
        const uint_t d1 = 10;
        const uint_t d21 = 10;
        const uint_t d22 = 20;
        const uint_t d31 = 5;
        const uint_t d32 = 7;
        const uint_t d33 = 15;

        vec1d v1(d1);
        vec2s v2(d21,d22);
        vec3b v3(d31,d32,d33);

        vec1d nv1(v1);
        vec2s nv2(v2);
        vec3b nv3(v3);

        check(nv1.dims[0], d1);
        check(v1.dims[0], d1);
        check(nv2.dims[0], d21);
        check(nv2.dims[1], d22);
        check(v2.dims[0], d21);
        check(v2.dims[1], d22);
        check(nv3.dims[0], d31);
        check(nv3.dims[1], d32);
        check(nv3.dims[2], d33);
        check(v3.dims[0], d31);
        check(v3.dims[1], d32);
        check(v3.dims[2], d33);

        check(nv1.size(), d1);
        check(v1.size(), d1);
        check(nv2.size(), d21*d22);
        check(v2.size(), d21*d22);
        check(nv3.size(), d31*d32*d33);
        check(v3.size(), d31*d32*d33);

        bool res = true;
        for (double& v : nv1) {
            res = res && v == 0.0;
        }
        check(res, true);
        res = true;
        for (std::string& v : nv2) {
            res = res && v == "";
        }
        check(res, true);
        res = true;
        for (bool& v : nv3) {
            res = res && v == false;
        }
        check(res, true);

        vec1d w1({0.0, 5.0, -2.0, dnan});
        vec2s w2({{"foo", "bar", "goo"}, {"blop", "plop", "flop"}});
        vec3b w3({{{true}, {false}}, {{false}, {true}}, {{true}, {true}}});

        vec1d nw1(w1);
        vec2s nw2(w2);
        vec3d nw3(w3);

        for (uint_t i : range(w1)) {
            check_float(nw1[i], w1[i]);
        }
        for (uint_t i : range(w2)) {
            check(nw2[i], w2[i]);
        }
        for (uint_t i : range(w3)) {
            check(nw3[i], w3[i]);
        }
    }

    print("> passed");
}

int main(int argc, char* argv[]) {
    print("Checking template metaprogramming");
    test_is_vec();
    test_vec_dim();
    test_vec_constructor();

    return 0;
}
