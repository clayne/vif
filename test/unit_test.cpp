#include <vif.hpp>

using namespace vif;

#define check(t, s) { \
    std::string st = to_string(t); \
    if (st == s) print("  checked: "+st); \
    else         print("  failed: "+std::string(#t)+" = "+st+" != "+s); \
    assert(st == s); \
}

int vif_main(int argc, char* argv[]) {
    double start = now();

    file::mkdir("out");

    {
        print("Checking template metaprogramming");
        static_assert(meta::is_vec<vec1d>::value, "failed meta::is_vec");
        static_assert(meta::is_vec<vec2s>::value, "failed meta::is_vec");
        static_assert(!meta::is_vec<float>::value, "failed meta::is_vec");
        static_assert(!meta::is_vec<std::vector<int>>::value, "failed meta::is_vec");
    }

    {
        print("Type checking on array indexation");
        static_assert(std::is_same<
            impl::vec_access::helper<true,false,3,float,int_t,int_t,int_t>::type,
            float&>::value, "wrong type of indexed array");
        static_assert(std::is_same<
            impl::vec_access::helper<true,true,3,float,int_t,int_t,int_t>::type,
            const float&>::value, "wrong type of indexed array");
        static_assert(std::is_same<
            impl::vec_access::helper<true,false,3,float,int_t,impl::placeholder_t,int_t>::type,
            vec<1,float*>>::value, "wrong type of indexed array");
        static_assert(std::is_same<
            impl::vec_access::helper<true,true,3,const float,int_t,impl::placeholder_t,int_t>::type,
            vec<1,const float*>>::value, "wrong type of indexed array");
        static_assert(std::is_same<
            impl::vec_access::helper<true,false,3,float,int_t,impl::placeholder_t,int_t>::type,
            vec<1,float*>>::value, "wrong type of indexed array");
        static_assert(std::is_same<
            impl::vec_access::helper<true,false,3,float,impl::placeholder_t,impl::placeholder_t,impl::placeholder_t>::type,
            vec<3,float*>>::value, "wrong type of indexed array");
        static_assert(std::is_same<
            impl::vec_access::helper<true,false,3,float,vec<1, int_t>,impl::placeholder_t,impl::placeholder_t>::type,
            vec<3,float*>>::value, "wrong type of indexed array");
        static_assert(std::is_same<
            impl::vec_access::helper<true,false,3,float,vec<1, int_t>,impl::placeholder_t,int_t>::type,
            vec<2,float*>>::value, "wrong type of indexed array");
        static_assert(std::is_same<
            impl::vec_access::helper<true,false,3,float,vec<1, int_t>,impl::placeholder_t,int_t>::type,
            vec<2,float*>>::value, "wrong type of indexed array");
        static_assert(std::is_same<
            impl::vec_access::helper<true,false,3,float,impl::repeated_value<3,impl::placeholder_t>>::type,
            vec<3,float*>>::value, "wrong type of indexed array");
        static_assert(std::is_same<
            impl::vec_access::helper<true,false,3,float,impl::repeated_value<2,impl::placeholder_t>,int_t>::type,
            vec<2,float*>>::value, "wrong type of indexed array");
        static_assert(std::is_same<
            impl::vec_access::helper<true,false,3,float,int_t,impl::repeated_value<2,impl::placeholder_t>>::type,
            vec<2,float*>>::value, "wrong type of indexed array");
    }

    {
        print("Default initialization of array");
        vec1u t(3);
        check(t, "{0, 0, 0}");
    }

    {
        print("Range");
        bool good = true;
        uint_t j = 0;
        for (uint_t i : range(10)) {
            good = good && i == j;
            ++j;
        }
        check(good, "1");
        check(j, "10");

        j = 2;
        for (uint_t i : range(2, 10)) {
            good = good && i == j;
            ++j;
        }
        check(good, "1");
        check(j, "10");

        j = 2;
        for (uint_t i : range(2, 10, 4)) {
            good = good && i == j;
            j += 2;
        }
        check(good, "1");
        check(j, "10");
    }

    {
        print("Index generation 'indgen'");
        vec1f v = indgen<float>(7);
        check(v, "{0, 1, 2, 3, 4, 5, 6}");

        print("Direct data access (single index)");
        check(v[1], "1");
        check(v(1), "1");
        print("Range index '_'");
        check(v[_], "{0, 1, 2, 3, 4, 5, 6}");
        print("Range index '3-_'");
        check(v[3-_], "{3, 4, 5, 6}");
        print("Range index '_-3'");
        check(v[_-3], "{0, 1, 2, 3}");
        print("Range index '2-_-4'");
        check(v[2-_-4], "{2, 3, 4}");

        print("Basic math");
        check(2*v, "{0, 2, 4, 6, 8, 10, 12}");
        check(pow(2,v), "{1, 2, 4, 8, 16, 32, 64}");

        print("'rgen' function");
        vec1d v1 = rgen(0, 5, 11);
        check(v1, "{0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5}");

        print("Index array");
        vec1u i = {5, 4, 3, 2};
        check((2*v)[i], "{10, 8, 6, 4}");
        check(2*v[i], "{10, 8, 6, 4}");
        v[i] = indgen<double>(4);
        check(v, "{0, 1, 3, 2, 1, 0, 6}");
        v[vec1u{4,5,6}] = v[vec1u{2,1,0}];
        check(v, "{0, 1, 3, 2, 3, 1, 0}");
        v[i] *= 2;
        check(v, "{0, 1, 6, 4, 6, 2, 0}");
        v[i] += indgen<double>(4);
        check(v, "{0, 1, 9, 6, 7, 2, 0}");
        v[vec1u{0,1,2,3,4,5,6}] = v[vec1u{6,5,4,3,2,1,0}];
        check(v, "{0, 2, 7, 6, 9, 1, 0}");
        vec1d tv = v[vec1u{1,2,3,4}][vec1u{1,2}];
        check(tv, "{7, 6}");
        v(1) = 3;
        check(v, "{0, 3, 7, 6, 9, 1, 0}");
        check(v(3), "6");
        v[vec1u{0,1,2,3,4,5,6}](0) = 1;
        check(v, "{1, 3, 7, 6, 9, 1, 0}");

        print("Negative indices");
        check(v[-1], "0");
    }

    {
        print("Boolean specialization");
        vec1b v = {true, false, true, false};
        bool& b1 = v[0];
        b1 = false;
        bool& b2 = v[1];
        b2 = true;
        check(v, "{0, 1, 1, 0}");
    }

    {
        print("Initialization list");
        vec1f tv = {44, 55, 66, 77};
        check(tv.dims, "{4}");
        check(tv.size(), "4");
        check(tv, "{44, 55, 66, 77}");
    }

    {
        print("String array");
        vec1s w = {"gn", "gs", "uds", "cosmos"};
        check(w, "{gn, gs, uds, cosmos}");
    }

    {
        print("Boolean logic");
        vec1i i = indgen<int_t>(5);
        vec1b b = (i == 3 || i == 1);
        check(b, "{0, 1, 0, 1, 0}");
        check(!b, "{1, 0, 1, 0, 1}");

        print("'where' function");
        vec1u id = where(b);
        check(id, "{1, 3}");

        print("'complement' function");
        vec1u cid = complement(b, id);
        check(cid, "{0, 2, 4}");

        print("'is_any_of' function");
        check(where(is_any_of(i, {3, 1})), "{1, 3}");
    }

    {
        print("'flatten' & 'reform' functions");
        vec2u v = {{1,2,3}, {4,5,6}};
        vec1u fv = flatten(v);
        check(fv.dims, "{"+to_string(v.size())+"}");
        check(fv, "{1, 2, 3, 4, 5, 6}");
        vec2u rv = reform(v, 3, 2);
        check(rv.dims, "{3, 2}");
        check(rv, "{1, 2, 3, 4, 5, 6}");
    }

    {
        print("'reverse' function");
        vec1i i = indgen<int_t>(5);
        check(reverse(i), "{4, 3, 2, 1, 0}");
    }

    {
        print("'remove' function");
        vec1i i = indgen<int_t>(10);
        vec1i di = remove(i, {1,2,5,6,7,9});
        check(di, "{0, 3, 4, 8}");
        check(di.dims, "{4}");
        vec2u u = {{0,1,2},{3,4,5},{6,7,8},{9,10,11},{12,13,14}};
        vec2u du = remove(u, {0,1,3});
        check(du, "{6, 7, 8, 12, 13, 14}");
        check(du.dims, "{2, 3}");
    }

    {
        print("2 dimensional array & multiple indices '(i,j)'");
        vec2d u = indgen<double>(3,2);
        check(u, "{0, 1, 2, 3, 4, 5}");
        check(u.dims, "{3, 2}");
        check(u(0,1), "1");
        check(u(1,1), "3");
        check(u(2,1), "5");

        u(1,0) = 9;
        u(2,1) = 10;
        check(u, "{0, 1, 9, 3, 4, 10}");

        vec2d tu = {{5,8},{4,5},{1,2}};
        check(tu.dims, "{3, 2}");
        check(tu.data.size(), "6");
        check(tu, "{5, 8, 4, 5, 1, 2}");
        check(tu(1,1), "5");
        tu(1-_-2,0-_-1) = {{1,2},{3,4}};
        check(tu, "{5, 8, 1, 2, 3, 4}");

        vec2i v = {{4,5,6,7}, {8,9,5,2}, {7,8,2,0}, {-1, -5, -6, -7}, {-4,1,-2,5}};
        check(v(_,_).dims, "{5, 4}");
        check(v(_,0).dims, "{5}");
        check(v(0,_).dims, "{4}");
        check(v(1-_-2,1-_-3).dims, "{2, 3}");
        check(v(vec1u{1,2},vec1u{1,2,3}).dims, "{2, 3}");
        check(v(vec1u{1,2},0).dims, "{2}");
        check(v(0,vec1u{1,2,3}).dims, "{3}");

        check(v(_,_).dims, "{5, 4}");
        check(v(_,0).dims, "{5}");
        check(v(0,_).dims, "{4}");
        check(v(vec1u{1,2},vec1u{1,2,3}).dims, "{2, 3}");
        check(v(vec1u{1,2},0).dims, "{2}");
        check(v(0,vec1u{1,2,3}).dims, "{3}");

        check(v(1,_), "{8, 9, 5, 2}");
        check(v(2,vec1u{1,2}), "{8, 2}");
        check(v(_,1), "{5, 9, 8, -5, 1}");

        v(2,_) = indgen<int_t>(4);
        check(v, "{4, 5, 6, 7, 8, 9, 5, 2, 0, 1, 2, 3, -1, -5, -6, -7, -4, 1, -2, 5}");
        v(_,2) = {-4, -4, -4, -4, -4};
        check(v, "{4, 5, -4, 7, 8, 9, -4, 2, 0, 1, -4, 3, -1, -5, -4, -7, -4, 1, -4, 5}");

        v(-1,_) = replicate(2,4);
        check(v, "{4, 5, -4, 7, 8, 9, -4, 2, 0, 1, -4, 3, -1, -5, -4, -7, 2, 2, 2, 2}");

        v(_,-1) = replicate(-2,5);
        check(v, "{4, 5, -4, -2, 8, 9, -4, -2, 0, 1, -4, -2, -1, -5, -4, -2, 2, 2, 2, -2}");

        check(flat_id(v,0,0), "0");
        check(flat_id(v,0,1), "1");
        check(flat_id(v,1,0), "4");
        check(flat_id(v,2,0), "8");
        check(flat_id(v,2,1), "9");
    }

    {
        print("Index conversion");
        vec2i v = indgen(2,3);
        for (uint_t i = 0; i < v.size(); ++i) {
            vec1u ids = mult_ids(v, i);
            check(v(ids[0], ids[1]), to_string(i));
        }
    }

    {
        print("FITS image loading");
        vec2d v; fits::header hdr;
        fits::read("data/image.fits", v, hdr);
        check(v.dims, "{161, 161}");
        check(float(v(80,79)), "0.0191503");
        check(hdr.size()/80, "9");

        print("Header functionalities");
        uint_t axis;
        check(fits::getkey(hdr, "NAXIS1", axis), "1");
        check(axis, "161");
        check(fits::setkey(hdr, "MYAXIS3", 12), "1");
        check(fits::setkey(hdr, "MYAXIS4", 44), "1");
        check(fits::getkey(hdr, "MYAXIS3", axis), "1");
        check(axis, "12");

        print("FITS image writing");
        fits::write("out/image_saved.fits", v, hdr);
        vec2d nv;
        fits::read("out/image_saved.fits", nv);
        check(count(nv != v) == 0, "1");
    }

    {
        print("FITS table loading");
        vec1i id1, id2, id3;
        fits::read_table("data/table.fits", ftable(id1, id2, id3));
        check(id1.size(), "79003");
        check(id2.size(), "79003");
        check(id3.size(), "79003");
        check(id1[indgen<int_t>(10)], "{43881, 43881, 43881, 43881, 43548, 43881, 43881, 43881, 43820, 43881}");

        print("FITS table writing");
        fits::write_table("out/table_saved.fits", ftable(id1, id2, id3));
        vec1i i1b = id1, i2b = id2, i3b = id3;
        fits::read_table("out/table_saved.fits", ftable(id1, id2, id3));
        check(where(id1 != i1b).empty(), "1");
        check(where(id2 != i2b).empty(), "1");
        check(where(id3 != i3b).empty(), "1");

        print("2-d column");
        vec2i test = indgen<double>(5,2);
        fits::write_table("out/2dtable.fits", ftable(test));

        vec2i otest = test;
        fits::read_table("out/2dtable.fits", ftable(test));

        check(where(test != otest).empty(), "1");

        print("Unsigned type");
        vec1u id = indgen(6);
        fits::write_table("out/utable.fits", ftable(id));

        vec1u oid = id;
        fits::read_table("out/utable.fits", ftable(id));

        check(where(id != oid).empty(), "1");

        print("String type");
        vec1s str = {"glouglou", "toto", "tat", "_gqmslk", "1", "ahahahaha"};
        fits::write_table("out/stable.fits", ftable(str));

        vec1s ostr = str;
        str = {};
        fits::read_table("out/stable.fits", ftable(str));
        check(where(str != ostr).empty(), "1");
    }

    {
        print("FITS table with non vector elements");
        int_t i = 5;
        float f = 0.2f;
        std::string s = "toto";
        fits::write_table("out/fits_single.fits", ftable(i, f, s));

        i = 0; f = 0; s = "";
        fits::read_table("out/fits_single.fits", ftable(i, f, s));
        check(i, "5");
        check(f, "0.2");
        check(s, "toto");
    }

    {
        print("'match' function");
        vec1i t1 = {4,5,6,7,8,9};
        vec1i t2 = {9,1,7,10,5};

        vec1u id1, id2;
        match(t1, t2, id1, id2);
        check(id1, "{1, 3, 5}");
        check(id2, "{4, 2, 0}");
    }

    {
        print("'histogram' function");
        vec1d t = {1,2,3,4,5,5,8,9,6,fnan,5,7,4};
        vec2i bins = {{0,2,5,8,10}, {2,5,8,10,12}};
        vec1u counts = histogram(t, bins);
        check(counts, "{1, 4, 5, 2, 0}");

        vec1d w = replicate(1.0, t.size());
        counts = histogram(t, w, bins);
        check(counts, "{1, 4, 5, 2, 0}");

        w[0] = 5;
        vec1d wcounts = histogram(t, w, bins);
        check(wcounts, "{5, 4, 5, 2, 0}");

        w[0] = 1; w[1] = 5;
        wcounts = histogram(t, w, bins);
        check(wcounts, "{1, 8, 5, 2, 0}");

        w[1] = 1; w[5] = 5;
        wcounts = histogram(t, w, bins);
        check(wcounts, "{1, 4, 9, 2, 0}");

        w[5] = 1; w[7] = 5;
        wcounts = histogram(t, w, bins);
        check(wcounts, "{1, 4, 5, 6, 0}");

        w[7] = 1; w[9] = 5;
        wcounts = histogram(t, w, bins);
        check(wcounts, "{1, 4, 5, 2, 0}");
    }

    {
        print("ASCII table loading");
        vec1i i1, i2;
        ascii::read_table("data/table.txt", i1, i2);
        check(i1, "{0, 1, 2, 3, 4}");
        check(i2, "{0, 1, 3, 5, 9}");

        vec1i i2b;
        ascii::read_table("data/table.txt", _, i2b);
        check(where(i2 != i2b).empty(), "1");

        vec2i ids;
        ascii::read_table("data/table.txt", ascii::columns(2, ids));
        check(where(i1 != ids(_,0)).empty(), "1");
        check(where(i2 != ids(_,1)).empty(), "1");

        vec2i i1c, i2c;
        ascii::read_table("data/table.txt", ascii::columns(1, i1c, i2c));
        check(where(i1 != i1c(_,0)).empty(), "1");
        check(where(i2 != i2c(_,0)).empty(), "1");

        vec2i i2d;
        ascii::read_table("data/table.txt", ascii::columns(1, _, i2d));
        check(where(i2 != i2d(_,0)).empty(), "1");
    }

    {
        print("File system functions");
        check(file::exists("unit_test.cpp"), "1");
        check(file::exists("unit_test_not_gonna_work.cpp"), "0");
        vec1s dlist = file::list_directories("../");
        print(dlist);
        check(where(dlist == "bin").empty(), "0");
        vec1s flist = file::list_files("./");
        print(flist);
        flist = file::list_files("../include/", "*.hpp");
        print(flist);
    }

    {
        print("file::get_directory and file::get_basename");
        check(file::get_directory("../somedir/someother/afile.cpp"), "../somedir/someother/");
        check(file::get_directory("../somedir/someother/"), "../somedir/");
        check(file::get_directory("afile.cpp"), "./");
        check(file::get_basename("../somedir/someother/afile.cpp"), "afile.cpp");
        check(file::get_basename("../somedir/someother/"), "someother");
        check(file::get_basename("afile.cpp"), "afile.cpp");
    }

    {
        print("'clamp' function");
        vec1f f = {-1,0,1,2,3,4,5,6,finf,fnan};
        check(clamp(f, 1, 4), "{1, 1, 1, 2, 3, 4, 4, 4, 4, nan}");
    }

    {
        print("'randomn', 'mean', 'median', 'percentile' functions");
        auto seed = make_seed(42);
        uint_t ntry = 20000;
        vec1d r = randomn(seed, ntry);

        double me = mean(r);
        print(me);
        check(abs(me) < 1.0/sqrt(ntry), "1");

        double med = median(r);
        print(med);
        check(abs(med) < 1.0/sqrt(ntry), "1");

        vec1d vv = {dnan, dnan, 2.0, dnan, dnan, 1.0, dnan, 3.0};
        check(median(vv), "2");

        double p1 = percentile(r, 0.158);
        double p2 = percentile(r, 0.842);
        vec1d pi = {p1, p2};
        print(pi);
        check(abs(p1 + 1.0) < 1.0/sqrt(ntry), "1");
        check(abs(p2 - 1.0) < 1.0/sqrt(ntry), "1");
        check(percentiles(r, 0.158, 0.842) == pi, "{1, 1}");
        check(percentile(vv, 0.16), "1");
        check(percentile(vv, 0.84), "3");
        check(percentiles(vv, 0.16, 0.5, 0.84), "{1, 2, 3}");
    }

    {
        print("'random_coin', 'fraction_of' functions");
        auto seed = make_seed(42);
        uint_t ntry = 10000;
        vec1b rnd = random_coin(seed, 0.3, ntry);
        check(abs(fraction_of(rnd)-0.3) < 1.0/sqrt(ntry), "1");
    }

    {
        print("'shuffle' function");
        auto seed = make_seed(42);
        vec1i i = {4,5,-8,5,2,0};
        vec1i si = shuffle(seed, i);
        vec1u id1, id2;
        match(i, si, id1, id2);
        check(i.size() == id1.size(), "1");
        check(si.size() == id2.size(), "1");
    }

    {
        print("'partial_mean' and 'partial_median' for 2-d array");
        vec2d v = {{1,2,3},{4,5,6},{7,8,9}};
        check(partial_mean(0,v), "{4, 5, 6}");
        check(partial_mean(1,v), "{2, 5, 8}");
        check(partial_median(0,v), "{4, 5, 6}");
        check(partial_median(1,v), "{2, 5, 8}");
        check(partial_min(0,v), "{1, 2, 3}");
        check(partial_min(1,v), "{1, 4, 7}");
        check(partial_max(0,v), "{7, 8, 9}");
        check(partial_max(1,v), "{3, 6, 9}");
    }

    {
        print("'partial_median' for 3-d array");
        vec3d v(21,21,11);
        v(_,_,0-_-4) = 0;
        v(_,_,6-_-10) = 1;
        v(_,_,5) = 0.5;

        vec2d m = partial_median(2,v);
        fits::write("out/median.fits", m);
    }

    {
        print("'reduce' function");
        vec2i v = {{1,2,3},{4,5,6}};
        vec1d mv = partial_mean(0, v);
        vec1d tmv = reduce(0, v, [](vec1d d) { return mean(d); });
        check(count(mv != tmv), "0");
    }

    {
        print("'replicate' function");
        vec1i v = replicate(3, 5);
        check(v.dims, "{5}");
        check(v, "{3, 3, 3, 3, 3}");
        v[indgen(2)] = indgen<int_t>(2)+1;
        vec2i v2 = replicate(v[indgen<int_t>(2)], 3);
        check(v2, "{1, 2, 1, 2, 1, 2}");
        check(v2.dims, "{3, 2}");
    }

    {
        print("'transpose' function");
        vec2f v = indgen<float>(3,2);
        vec2f tv = transpose(v);
        uint_t nok = 0;
        for (uint_t i = 0; i < 3; ++i)
        for (uint_t j = 0; j < 2; ++j) {
            nok += v(i,j) != tv(j,i);
        }
        check(nok, "0");
    }

    {
        print("'circular_mask' function");
        vec1u dim = {51,41};
        vec2d px = replicate(indgen<double>(dim[1]), dim[0]);
        vec2d py = transpose(replicate(indgen<double>(dim[0]), dim[1]));
        fits::write("out/px.fits", px);
        fits::write("out/py.fits", py);

        vec2d m = astro::circular_mask({{51,51}}, 25, 25, 8);
        fits::write("out/circular.fits", m);
    }

    {
        print("'enlarge' function");
        vec2d v = replicate(3.1415, 5, 3);
        v = astro::enlarge(v, 5, 1.0);
        fits::write("out/enlarge.fits", v);
    }

    {
        print("'subregion' function");
        vec2i v = indgen<int_t>(5,5);
        fits::write("out/sub0.fits", v);

        vec2i s = astro::subregion(v, {0,0,4,4});
        vec2i tv = v;
        check(count(s != tv) == 0, "1");
        fits::write("out/sub1.fits", s);

        s = astro::subregion(v, {0,1,4,3});
        tv = {{1,2,3},{6,7,8},{11,12,13},{16,17,18},{21,22,23}};
        check(count(s != tv) == 0, "1");
        fits::write("out/sub2.fits", s);

        s = astro::subregion(v, {1,-1,3,5});
        tv = {{0,5,6,7,8,9,0},{0,10,11,12,13,14,0},{0,15,16,17,18,19,0}};
        check(count(s != tv) == 0, "1");
        fits::write("out/sub4.fits", s);
    }

    {
        print("'translate' function");
        vec2d test(51,51);
        test(25,25) = 1.0;

        vec2d trans = astro::translate(test, 2, 0);

        vec1u mid = mult_ids(trans, max_id(trans));
        check(mid[0], "27");
        check(mid[1], "25");

        trans = astro::translate(trans, -2, 0);
        mid = mult_ids(trans, max_id(trans));
        check(mid[0], "25");
        check(mid[1], "25");
    }

    {
        print("'push_back' function");
        vec1i i = {0,1,2,4,-1};
        i.push_back(3);
        check(i, "{0, 1, 2, 4, -1, 3}");

        vec2i j = {{1, 2}, {3, 4}};
        j.push_back({5, 6});
        check(j.dims, "{3, 2}");
        check(j, "{1, 2, 3, 4, 5, 6}");
    }

    {
        print("'append' & 'prepend' functions");
        vec1i v = {0, 5, 2};
        vec1i w = {4, 1, 3};
        append(v, w);
        check(v, "{0, 5, 2, 4, 1, 3}");
        prepend(v, w);
        check(v, "{4, 1, 3, 0, 5, 2, 4, 1, 3}");
    }

    {
        print("'append' for 2-d array");
        vec2i v = {{0,1,2}, {5,6,8}};
        vec2i w = {{5,5,5}};
        append<0>(v, w);
        check(v(0,_), "{0, 1, 2}");
        check(v(1,_), "{5, 6, 8}");
        check(v(2,_), "{5, 5, 5}");
    }

    {
        print("'shift' function");
        vec1i v = indgen<int_t>(6);
        check(shift(v, -2), "{2, 3, 4, 5, 0, 1}");
        check(shift(v, +2), "{4, 5, 0, 1, 2, 3}");
    }

    {
        print("Calculus functions");
        vec1d x = {0,1,2};
        vec1d y = {0,1,0};
        check(integrate(x, y), "1");
        check(integrate(x, y, 0, 2), "1");
        check(integrate(x, y, 0, 1), "0.5");
        check(integrate(x, y, 1, 2), "0.5");
        check(integrate(x, y, 0.5, 1.5), "0.75");
        check(integrate(x, y, 1.25, 1.75), "0.25");

        vec2d x2 = {{0,1,2,3}, {1,2,3,4}};
        vec1d y2 = {1,1,1,1};
        check(integrate(x2, y2), "4");
        check(integrate(x2, y2, 0, 4), "4");
        check(integrate(x2, y2, 0, 3), "3");
        check(integrate(x2, y2, 0, 3.5), "3.5");
        check(integrate(x2, y2, 0.5, 4.0), "3.5");
        check(integrate(x2, y2, 0.5, 3.5), "3");
        check(integrate(x2, y2, 0.5, 1.5), "1");
        check(integrate(x2, y2, 1.25, 1.75), "0.5");

        x = 0.5*3.14159265359*indgen<double>(30)/29.0;
        y = cos(x);
        check(abs(1.0 - integrate(x,y)) < 0.001, "1");

        check(float(integrate_func([](float t) -> float {
                return (2.0/sqrt(3.14159))*exp(-t*t);
            }, 0.0, 1.0)), to_string(float(erf(1.0)))
        );
    }

    {
        print("Matrix 'invert', 'product', 'make_identity'");
        matrix::mat2d a = {
            {1.000, 2.000, 3.000},
            {4.000, 1.000, 6.000},
            {7.000, 8.000, 1.000}
        };

        matrix::mat2d id = {{1,0,0},{0,1,0},{0,0,1}};

#ifndef NO_LAPACK
        matrix::mat2d i;
        check(matrix::invert(a,i), "1");
        matrix::mat2d tid = a*i;
        check(rms((tid - id).base) < 1e-10, "1");
#endif

        matrix::mat<int_t> sq = {{1,1,1},{1,1,1},{1,1,1}};
        matrix::diagonal(sq) *= 5;
        check(sq.base, "{5, 1, 1, 1, 5, 1, 1, 1, 5}");

        matrix::mat2d id2 = matrix::make_identity(3);
        check(rms((id2 - id).base) < 1e-10, "1");

        check(rms((id*a - a).base) < 1e-10, "1");
    }

    {
        print("Matrix 'make_translation', 'make_scale', 'make_rotation'");
        vec1d p = {5, 2, 1};

        matrix::mat2d tm = matrix::make_identity(3);

        matrix::mat2d m = matrix::make_translation(2.0,3.0);
        vec1d tp = m*p;
        tm = m*tm;
        check(tp, "{7, 5, 1}");

        m = matrix::make_scale(2.0,3.0);
        tp = m*tp;
        tm = m*tm;
        check(tp, "{14, 15, 1}");

        m = matrix::make_rotation(dpi/2);
        tp = m*tp;
        tm = m*tm;
        check(tp, "{-15, 14, 1}");

        check(tm*p, "{-15, 14, 1}");
    }

    {
        print("Matrix 'invert_symmetric' function");
        matrix::mat2d alpha = matrix::make_identity(2);
        alpha(1,0) = alpha(0,1) = 2;
        matrix::mat2d talpha = alpha;
        check(matrix::inplace_invert(talpha), "1");
        check(matrix::inplace_invert_symmetric(alpha), "1");
        matrix::inplace_symmetrize(alpha);
        check(count(abs((alpha - talpha).base) > 1e-6), "0");
    }

    {
        print("Matrix 'solve_symmetric' function");
        vec1d beta = {1,2,3,4};
        matrix::mat2d alpha = matrix::make_identity(4);
        check(matrix::inplace_solve_symmetric(alpha, beta), "1");
        check(beta, "{1, 2, 3, 4}");

        // x + 2*y = b1
        // 2*x + y = b2
        // -> x = (2*b2 - b1)/3
        // -> y = (2*b1 - b2)/3
        beta = {1,2};
        vec1d tbeta = beta;
        alpha = matrix::make_identity(2);
        alpha(1,0) = alpha(0,1) = 2;
        matrix::mat2d talpha = alpha;
        check(matrix::inplace_solve_symmetric(alpha, beta), "1");
        check(count(abs(beta - vec1d{1, 0}) > 1e-6), "0");

        vec1d ubeta = talpha*beta;
        check(count(abs(ubeta - tbeta) > 1e-6), "0");

        matrix::inplace_invert(talpha);
        tbeta = talpha*tbeta;
        check(count(abs(beta - tbeta) > 1e-6), "0");
    }

#ifndef NO_LAPACK
    {
        print("Matrix 'eigen_symmetric' function");
        matrix::mat2d alpha = {
            {1.0, -0.1, 0.1},
            {-0.1, 1.3, 0.1},
            {0.1,  0.1, 1.5}
        };

        uint_t n = alpha.dims[0];

        vec1d vals;
        matrix::mat2d vecs;
        check(matrix::eigen_symmetric(alpha, vals, vecs), "1");
        for (uint_t i : range(n)) {
            vec1d u1 = vecs(i,_);
            vec1d u2 = (alpha*u1)/u1;
            for (uint_t j : range(n-1)) {
                check(abs(u2[j] - u2[j+1]) < 1e-6, "1");
            }
            check(abs(u2[0] - vals[i]) < 1e-6, "1");
        }
    }
#endif

    {
        print("'linfit' function");
        vec1d x = indgen<double>(5);
        vec1d y = 3.1415*x*x - 12.0*x;

        auto fr = linfit(y, 1.0, 1.0, x, x*x);
        if (fr.success) {
            check(abs(fr.params(0)) < 1e-6, "1");
            check(fr.params(1), "-12");
            check(fr.params(2), "3.1415");
        } else {
            print("failure...");
            print("param: ", fr.params);
            print("error: ", fr.errors);
            print("chi2: ", fr.chi2);
            check(fr.success, "1");
        }
    }

    {
        print("'linfit_pack' function");
        vec1d tx = indgen<double>(5);
        vec1d y = 3.1415*tx*tx - 12.0*tx;
        vec1d ye = y*0 + 1;
        vec2d x(3,5);
        x(0,_) = 1.0;
        x(1,_) = tx;
        x(2,_) = tx*tx;

        auto fr = linfit_pack(y, ye, x);
        if (fr.success) {
            check(abs(fr.params(0)) < 1e-6, "1");
            check(fr.params(1), "-12");
            check(fr.params(2), "3.1415");
        } else {
            print("failure...");
            print("param: ", fr.params);
            print("error: ", fr.errors);
            print("chi2: ", fr.chi2);
            check(fr.success, "1");
        }
    }

    {
        print("'linfit' function doing PSF fitting");
        vec2d img, psf;
        fits::read("data/stack1.fits", img);
        fits::read("data/psf.fits", psf);

        auto fr = linfit(img, 1.0, 1.0, psf);
        if (fr.success) {
            check(float(fr.params(0)), "21.6113");
            check(float(fr.params(1)), "90.4965");
        } else {
            print("params: ", fr.params);
            print("errors: ", fr.errors);
            print("chi2: ", fr.chi2);
            assert(false);
        }

        img -= psf*fr.params(1);
        fits::write("out/residual.fits", img);
    }

    {
        print("'sort' function");
        vec1d v = {6,2,4,3,1,5};
        vec1i sid = sort(v);
        check(sid, "{4, 1, 3, 2, 5, 0}");
        check(v[sid], "{1, 2, 3, 4, 5, 6}");

        v = {dnan,2,4,3,1,5};
        sid = sort(v);
        vec1d tv = v[sid];
        check(tv[where(is_finite(tv))], "{1, 2, 3, 4, 5}");

        v = {2,4,3,1,5,dnan};
        sid = sort(v);
        tv = v[sid];
        check(tv[where(is_finite(tv))], "{1, 2, 3, 4, 5}");

        v = {6,6,4,6,4,1,5};
        sid = sort(v);
        check(v[sid], "{1, 4, 4, 5, 6, 6, 6}");
    }

    {
        vec2i v = {{5,0}, {4,1}, {1,2}, {2,3}};
        vec1i sid = sort(v(_,0));
        v(_,_) = v(sid,_);
        check(v, "{1, 2, 2, 3, 4, 1, 5, 0}");
    }

    {
        print("'unique_*' functions");
        vec1i v = {0,1,1,1,2,2,3,5,5,6};
        vec1u id = unique_ids_sorted(v);
        check(id, "{0, 1, 4, 6, 7, 9}");

        auto seed = make_seed(42);
        inplace_shuffle(seed, v);
        vec1i id2 = unique_values(v);
        check(id2, "{0, 1, 2, 3, 5, 6}");
    }

    {
        print("'min', 'max'");
        vec1f f = {2.0f, 3.0f, -1.0f, 0.0f, 4.45f, -1.5f};
        check(min(f), "-1.5"); check(max(f), "4.45");
        f = {2.0f, 3.0f, -1.0f, 0.0f, 4.45f, fnan, -1.5f};
        check(min(f), "-1.5"); check(max(f), "4.45");
        f = {2.0f, fnan, -1.0f, 0.0f, 4.45f, fnan, -1.5f};
        check(min(f), "-1.5"); check(max(f), "4.45");
        f = {2.0f, 3.0f, -1.0f, 0.0f, 4.45f, finf, -1.5f};
        check(min(f), "-1.5"); check(!is_finite(max(f)), "1");
        f = {2.0f, 3.0f, -1.0f, 0.0f, 4.45f, -finf, -1.5f};
        check(!is_finite(min(f)), "1"); check(max(f), "4.45");
        vec1f f2 = {1.0f, 5.0f, -1.0f, fnan, 12.0f, 6.0, -1e6};
        check(max(f, f2), "{2, 5, -1, 0, 12, 6, -1.5}");
        check(min(f, f2), "{1, 3, -1, 0, 4.45, -inf, -1e+06}");
        f = {fnan, fnan};
        check(max(f), to_string(fnan));
    }

    {
        print("'min_id', 'max_id'");
        vec1i i = {5,2,4,9,6,-5,2,-3};
        uint_t mi = min_id(i);
        uint_t ma = max_id(i);
        check(mi, "5");
        check(ma, "3");
    }

    {
        print("'replace' & 'split'");
        vec1s s = {"/home/dir/", "/bin//ds9", "ls"};
        s = replace(s, "/", ":");
        check(s, "{:home:dir:, :bin::ds9, ls}");

        vec1s t = split(s[0], ":");
        check(t, "{, home, dir, }");
    }

    {
        print("string 'match' regex");
        vec1s s = {"[u]=5", "[v]=2", "plop", "3=[c]", "5]=[b"};
        check(regex_match(s, "\\[.\\]"), "{1, 1, 0, 1, 0}");
    }

    {
        print("Cosmology functions");
        auto cosmo = astro::cosmo_wmap();
        check(float(astro::lumdist(0.5, cosmo)), "2863.03");
        check(float(astro::lookback_time(0.5, cosmo)), "5.09887");

        vec1d v = rgen(0.0, 12.0, 1500);
        vec1d r = astro::lumdist(v, cosmo);
        vec1d r1 = astro::lumdist(v[0-_-750], cosmo);
        append(r1, astro::lumdist(v[751-_-1499], cosmo));
        check(max(abs(r - r1)/r1) < 1e-5, "1");
    }

    {
        print("'interpolate', 'lower_bound' & 'upper_bound' functions");
        vec1f y = {0, 1, 2, 3, 4, 5};
        vec1f x = {0, 1, 2, 3, 4, 5};
        vec1f nx = {0.2, 0.5, 1.6, -0.5, 12};
        vec1f i = interpolate(y, x, nx);
        check(count(i != nx), "0");

        i = indgen<float>(10);

        check(lower_bound(i, 0.5) == 0, "1");
        check(upper_bound(i, 0.5) == 1, "1");

        check(lower_bound(i, -0.1) == npos, "1");
        check(lower_bound(i, 0.0) == 0, "1");
        check(lower_bound(i, 0.1) == 0, "1");

        check(upper_bound(i, -0.1) == 0, "1");
        check(upper_bound(i, 0.0) == 1, "1");
        check(upper_bound(i, 0.1) == 1, "1");

        check(lower_bound(i, 8.9) == 8, "1");
        check(lower_bound(i, 9.0) == 9, "1");
        check(lower_bound(i, 9.1) == 9, "1");

        check(upper_bound(i, 8.9) == 9, "1");
        check(upper_bound(i, 9.0) == npos, "1");
        check(upper_bound(i, 9.1) == npos, "1");
    }

    {
        print("'bilinear' function");
        vec2d m = {{0,2},{0,2}};
        check(abs(bilinear(m, 0.5, 0.5) - 1.0) < 1e-5, "1");
        check(abs(bilinear(m, 0.0, 0.5) - 1.0) < 1e-5, "1");
        check(abs(bilinear(m, 1.0, 0.5) - 1.0) < 1e-5, "1");
        check(abs(bilinear(m, 0.5, 0.25) - 0.5) < 1e-5, "1");
        check(abs(bilinear(m, 0.0, 0.25) - 0.5) < 1e-5, "1");
        check(abs(bilinear(m, 1.0, 0.25) - 0.5) < 1e-5, "1");

        m = {{0,0},{2,2}};
        check(abs(bilinear(m, 0.5, 0.5) - 1.0) < 1e-5, "1");
        check(abs(bilinear(m, 0.5, 0.0) - 1.0) < 1e-5, "1");
        check(abs(bilinear(m, 0.5, 1.0) - 1.0) < 1e-5, "1");
        check(abs(bilinear(m, 0.25, 0.5) - 0.5) < 1e-5, "1");
        check(abs(bilinear(m, 0.25, 0.0) - 0.5) < 1e-5, "1");
        check(abs(bilinear(m, 0.25, 1.0) - 0.5) < 1e-5, "1");

        m = {{0,1},{1,1}};
        print(bilinear(m, 0.5, 0.5));
        check(abs(bilinear(m, 0.5, 0.5) - 0.75) < 1e-5, "1");
    }

    {
        print("'uJy2lsun' & 'lsun2uJy' functions");
        vec1d jy = {1000.0};
        double z = 1.0;
        double d = astro::lumdist(z, astro::cosmo_wmap());
        double lam_obs = 160.0;
        auto lsun = astro::uJy2lsun(z, d, lam_obs, jy);
        double lam_rf = lam_obs/(1.0 + z);
        check(abs(jy - astro::lsun2uJy(z, d, lam_rf, lsun)) < 1e-5, "{1}");
    }

#ifndef NO_REFLECTION
    {
        print("Reflection: save & write a structure");
        struct tmp_t {
            vec1d f = indgen<double>(3);
            vec1i s = {5,4,2,3,1};
            struct {
                vec1i u = {1,2};
                vec1i v = {3,4};
            } t;
        };

        tmp_t tmp;
        tmp.f = {5,4,8,9};
        tmp.s = {1,2};
        std::swap(tmp.t.u, tmp.t.v);

        tmp_t tmp2;

        fits::write_table("out/reflex_tbl.fits", tmp);
        fits::read_table("out/reflex_tbl.fits", tmp2);

        check(tmp2.f, to_string(tmp.f));
        check(tmp2.s, to_string(tmp.s));
        check(tmp2.t.u, to_string(tmp.t.u));
        check(tmp2.t.v, to_string(tmp.t.v));
    }

    {
        print("Reflection: merge two structures");
        struct {
            int_t i = 0;
            float j = 0;
            struct {
                float k = 0;
                std::string v = "";
                int_t w = 0;
            } t;
        } tmp1;

        struct {
            int_t i = 5;
            struct {
                std::string v = "toto";
            } t;
            struct {
                int_t z = 12;
            } p;
        } tmp2;


        struct ut {
            struct utt { int_t i, j; } t;
        } tmp3;

        reflex::merge_elements(tmp1, tmp2);

        check(tmp1.i, "5");
        check(tmp1.j, "0");
        check(tmp1.t.k, "0");
        check(tmp1.t.v, "toto");
        check(tmp1.t.w, "0");
    }

    {
        print("Reflection: copy a structure");
        struct tmp {
            int i = 5;
            struct {
                int k = -4;
            } j;
        } tmp1;

        tmp tmp2 = tmp1;
        tmp2.i = 6;
        tmp2.j.k = 12;

        check(tmp2.i, "6");
        check(tmp2.j.k, "12");
        check(reflex::seek_name(tmp1.j.k), "tmp.j.k");
        check(reflex::seek_name(tmp2.j.k), "tmp.j.k");
    }
#endif

    {
        print("Convex hull");
        struct {
            vec1d ra, dec;
            vec1i hull;
        } cat;

        fits::read_table("data/sources.fits", ftable(cat.ra, cat.dec));
        auto hull = build_convex_hull(cat.ra, cat.dec);

        vec1d tra = 1.2*(cat.ra - mean(cat.ra)) + mean(cat.ra);
        vec1d tdec = 1.2*(cat.dec - mean(cat.dec)) + mean(cat.dec);

        vec1b inhull = in_convex_hull(tra, tdec, hull);
        vec1d hdist = convex_hull_distance(tra, tdec, hull);
        fits::write_table("out/hull.fits", ftable(tra, tdec, inhull, hdist, hull.x, hull.y));

        print(astro::field_area_hull(hull));

        check(in_convex_hull(mean(cat.ra), mean(cat.dec), hull), "1");
        check(in_convex_hull(2*mean(cat.ra), 2*mean(cat.dec), hull), "0");
    }

    {
        print("sex2deg & deg2sex");
        std::string sra = "3:32:21.129";
        std::string sdec = "-27:56:26.674";

        double ra, dec;
        astro::sex2deg(sra, sdec, ra, dec);

        check(format::precision(ra, 12), "53.0880375");
        check(format::precision(dec, 12), "-27.9407427778");

        std::string sra2, sdec2;
        astro::deg2sex(ra, dec, sra2, sdec2);
        check(sra2, sra);
        check(sdec2, sdec);
    }

    print("\nall tests passed in "+time_str(now() - start)+"!\n");

    return 0;
}
