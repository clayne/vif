#ifndef PHYPP_ASTRO_IMAGE_HPP
#define PHYPP_ASTRO_IMAGE_HPP

#include "phypp/core/vec.hpp"
#include "phypp/core/error.hpp"
#include "phypp/core/range.hpp"
#include "phypp/utility/generic.hpp"
#include "phypp/math/base.hpp"
#include "phypp/math/fourier.hpp"

namespace phypp {
namespace astro {
    template<typename Type>
    vec<2,meta::rtype_t<Type>> enlarge(const vec<2,Type>& v, const std::array<uint_t,4> upix,
        const meta::rtype_t<Type>& def = 0.0) {

        vec<2,meta::rtype_t<Type>> r = replicate(def,
            v.dims[0]+upix[0]+upix[2], v.dims[1]+upix[1]+upix[3]);

        for (uint_t x : range(v.dims[0]))
        for (uint_t y : range(v.dims[1])) {
            r.safe(x+upix[0],y+upix[1]) = v.safe(x,y);
        }

        return r;
    }

    template<typename Type>
    vec<2,meta::rtype_t<Type>> enlarge(const vec<2,Type>& v, uint_t upix,
        const meta::rtype_t<Type>& def = 0.0) {
        return enlarge(v, {{upix, upix, upix, upix}}, def);
    }

    template<typename Type>
    vec<2,meta::rtype_t<Type>> shrink(const vec<2,Type>& v, const std::array<uint_t,4> upix) {
        if (upix[0]+upix[2] >= v.dims[0] || upix[1]+upix[3] >= v.dims[1]) {
            return vec<2,meta::rtype_t<Type>>();
        }

        vec<2,meta::rtype_t<Type>> r(v.dims[0]-upix[0]-upix[2], v.dims[1]-upix[1]-upix[3]);

        for (uint_t x : range(r.dims[0]))
        for (uint_t y : range(r.dims[1])) {
            r.safe(x,y) = v.safe(x+upix[0],y+upix[1]);
        }

        return r;
    }

    template<typename Type>
    vec<2,meta::rtype_t<Type>> shrink(const vec<2,Type>& v, uint_t upix) {
        return shrink(v, {{upix, upix, upix, upix}});
    }

    // Get a sub region 'reg' inside an image 'v', with reg = {x0, y0, x1, y1} (inclusive)
    // assuming that "x" and "y" correspond to v(x,y).
    // The sub region is returned as two index vectors, the first containing indices in the
    // image 'v', and the second containing indices in the region 'reg'. The number of
    // returned indices may be smaller than the size of the requested region, if a fraction of
    // the region falls out of the image boundaries. In particular, the index vectors will be
    // empty if there is no overlap between the image and the requested region.
    template<typename TypeV, typename TypeR = int_t>
    void subregion(const vec<2,TypeV>& v, const vec<1,TypeR>& reg, vec1u& rv, vec1u& rr) {
        phypp_check(reg.size() == 4, "invalid region parameter "
            "(expected 4 components, got ", reg.size(), ")");

        int_t nvx = v.dims[0], nvy = v.dims[1];
        int_t nx = reg.safe[2]-reg.safe[0]+1, ny = reg.safe[3]-reg.safe[1]+1;

        vec1i vreg = reg;
        vec1i sreg = {0,0,nx-1,ny-1};

        // Early exit when not covered
        if (reg.safe[0] >= nvx || reg.safe[2] < 0 || reg.safe[0] > reg.safe[2] ||
            reg.safe[1] >= nvy || reg.safe[3] < 0 || reg.safe[1] > reg.safe[3]) {
            rv.clear(); rr.clear();
            return;
        }

        if (reg.safe[0] < 0) {
            vreg.safe[0] = 0;
            sreg.safe[0] += 0 - reg.safe[0];
        }
        if (reg.safe[2] >= nvx) {
            vreg.safe[2] = nvx-1;
            sreg.safe[2] -= reg.safe[2] - (nvx-1);
        }

        if (reg.safe[1] < 0) {
            vreg.safe[1] = 0;
            sreg.safe[1] += 0 - reg.safe[1];
        }
        if (reg.safe[3] >= nvy) {
            vreg.safe[3] = nvy-1;
            sreg.safe[3] -= reg.safe[3] - (nvy-1);
        }

        int_t nnx = vreg.safe[2]-vreg.safe[0]+1;
        int_t nny = vreg.safe[3]-vreg.safe[1]+1;
        uint_t npix = nnx*nny;

        rv.resize(npix);
        rr.resize(npix);
        for (uint_t i : range(npix)) {
            rv.safe[i] = (i%nny) + vreg.safe[1] + (i/nny + vreg.safe[0])*nvy;
            rr.safe[i] = (i%nny) + sreg.safe[1] + (i/nny + sreg.safe[0])*ny;
        }
    }

    template<typename TypeV, typename TypeR = int_t>
    typename vec<2,TypeV>::effective_type subregion(const vec<2,TypeV>& v,
        const vec<1,TypeR>& reg, const typename vec<2,TypeV>::rtype& def = 0.0) {

        vec1u rr, rs;
        subregion(v, reg, rr, rs);

        int_t nx = reg(2)-reg(0)+1, ny = reg(3)-reg(1)+1;
        vec<2,meta::rtype_t<TypeV>> sub = replicate(meta::rtype_t<TypeV>(def), nx, ny);

        sub.safe[rs] = v.safe[rr];

        return sub;
    }

    template<typename TypeV>
    typename vec<2,TypeV>::effective_type translate(const vec<2,TypeV>& v, double dx, double dy,
        typename vec<2,TypeV>::rtype def = 0.0) {

        vec<2,meta::rtype_t<TypeV>> trs(v.dims);
        for (uint_t x : range(v.dims[0]))
        for (uint_t y : range(v.dims[1])) {
            trs.safe(x,y) = bilinear_strict(v, x - dx, y - dy, def);
        }

        return trs;
    }

    template<typename TypeV>
    typename vec<2,TypeV>::effective_type flip_x(const vec<2,TypeV>& v) {
        auto r = v.concretise();
        for (uint_t y : range(v.dims[0]))
        for (uint_t x : range(v.dims[1]/2)) {
            std::swap(r.safe(y,x), r.safe(y,v.dims[1]-1-x));
        }

        return r;
    }

    template<typename TypeV>
    typename vec<2,TypeV>::effective_type flip_y(const vec<2,TypeV>& v) {
        auto r = v.concretise();
        for (uint_t y : range(v.dims[0]/2))
        for (uint_t x : range(v.dims[1])) {
            std::swap(r.safe(y,x), r.safe(v.dims[0]-1-y,x));
        }

        return r;
    }

    template<typename TypeV, typename TypeD = double>
    typename vec<2,TypeV>::effective_type scale(const vec<2,TypeV>& v, double factor,
        typename vec<2,TypeV>::rtype def = 0.0) {

        auto r = v.concretise();

        for (int_t y : range(v.dims[0]))
        for (int_t x : range(v.dims[1])) {
            r.safe(uint_t(y),uint_t(x)) = bilinear_strict(v,
                (y-int_t(v.dims[0]/2))/factor + v.dims[0]/2,
                (x-int_t(v.dims[1]/2))/factor + v.dims[1]/2,
                def
            );
        }

        return r;
    }

    template<typename TypeV, typename TypeD = double>
    typename vec<2,TypeV>::effective_type rotate(const vec<2,TypeV>& v, double angle,
        TypeD def = 0.0) {

        auto r = v.concretise();

        double ca = cos(angle*dpi/180.0);
        double sa = sin(angle*dpi/180.0);

        for (int_t y : range(v.dims[0]))
        for (int_t x : range(v.dims[1])) {
            double dy = (y-int_t(v.dims[0]/2));
            double dx = (x-int_t(v.dims[1]/2));
            r.safe(uint_t(y),uint_t(x)) = bilinear_strict(v,
                dy*ca - dx*sa + v.dims[0]/2,
                dx*ca + dy*sa + v.dims[1]/2,
                def
            );
        }

        return r;
    }

    template<typename T>
    vec<2,T> shift(vec<2,T> v, int_t tsx, int_t tsy) {
        tsx = (-tsx) % int_t(v.dims[0]);
        tsy = (-tsy) % int_t(v.dims[1]);
        if (tsx < 0) tsx = int_t(v.dims[0])+tsx;
        if (tsy < 0) tsy = int_t(v.dims[1])+tsy;
        uint_t sx = tsx, sy = tsy;

        if (sy != 0) {
            for (uint_t ix : range(v.dims[0])) {
                auto st = v.stride(ix,_);
                std::rotate(st.begin(), st.begin() + sy, st.end());
            }
        }

        if (sx != 0) {
            for (uint_t iy : range(v.dims[1])) {
                auto st = v.stride(_,iy);
                std::rotate(st.begin(), st.begin() + sx, st.end());
            }
        }

        return v;
    }

    inline vec2d circular_mask(const std::array<uint_t,2>& dims, double radius, double x, double y) {
        vec2d m(dims);

        phypp_check(radius >= 0, "radius must be a positive number");

        // Identify the needed region
        if (x+radius > 0 && y+radius > 0) {
            uint_t x0 = floor(x - radius) > 0         ? floor(x - radius) : 0;
            uint_t y0 = floor(y - radius) > 0         ? floor(y - radius) : 0;
            uint_t x1 = ceil(x + radius)  < dims[0]-1 ? ceil(x + radius)  : dims[0]-1;
            uint_t y1 = ceil(y + radius)  < dims[1]-1 ? ceil(y + radius)  : dims[1]-1;

            radius *= radius;
            for (uint_t ix = x0; ix <= x1; ++ix)
            for (uint_t iy = y0; iy <= y1; ++iy) {
                m.safe(ix,iy) = 0.25*(
                    (sqr(ix-0.5 - x) + sqr(iy-0.5 - y) <= radius) +
                    (sqr(ix+0.5 - x) + sqr(iy-0.5 - y) <= radius) +
                    (sqr(ix+0.5 - x) + sqr(iy+0.5 - y) <= radius) +
                    (sqr(ix-0.5 - x) + sqr(iy+0.5 - y) <= radius)
                );
            }
        }

        return m;
    }

    inline vec2d circular_mask(const std::array<uint_t,2>& dims, double radius) {
        return circular_mask(dims, radius, dims[0]/2.0, dims[1]/2.0);
    }

    template<typename Type>
    vec<1, meta::rtype_t<Type>> radial_profile(const vec<2,Type>& img, uint_t npix) {
        // TODO: (optimization) really...
        vec<1, meta::rtype_t<Type>> res(npix);
        uint_t hsx = img.dims[0]/2;
        uint_t hsy = img.dims[1]/2;
        res[0] = img(hsx,hsy);
        for (uint_t i : range(1u, npix)) {
            vec2d mask = circular_mask(img.dims, double(hsx), double(hsy), i)*
                (1.0 - circular_mask(img.dims, double(hsx), double(hsy), i-1));
            res.safe[i] = total(mask*img)/total(mask);
        }

        return res;
    }

    template<typename F>
    auto generate_img(const std::array<uint_t,2>& dims, F&& expr) -> vec<2,decltype(expr(0,0))> {
        vec<2,decltype(expr(0,0))> img(dims);
        for (uint_t x : range(img.dims[0]))
        for (uint_t y : range(img.dims[1])) {
            img.safe(x,y) = expr(x,y);
        }

        return img;
    }

    inline vec2d gaussian_profile(const std::array<uint_t,2>& dims, double sigma) {
        double norm = 1.0/(2.0*dpi*sqr(sigma));
        return generate_img(dims, [&](int_t x, int_t y) {
            return norm*exp(
                -(sqr(x - int_t(dims[0]/2)) + sqr(y - int_t(dims[1]/2)))/(2.0*sqr(sigma))
            );
        });
    }

    // Perform the convolution of two 2D arrays, assuming the second one is the kernel.
    // Naive loop implementation: this is slow but simple and reliable.
    template<typename TypeY1, typename TypeY2>
    auto convolve2d_naive(const vec<2,TypeY1>& map, const vec<2,TypeY2>& kernel) ->
        vec<2,decltype(map[0]*kernel[0])> {
        phypp_check(kernel.dims[0]%2 == 1 && kernel.dims[1]%2 == 1,
            "kernel must have odd dimensions (", kernel.dims, ")");

        uint_t hxsize = kernel.dims[0]/2;
        uint_t hysize = kernel.dims[1]/2;

        vec<2,decltype(map[0]*kernel[0])> r(map.dims);
        for (uint_t kx = 0; kx < kernel.dims[0]; ++kx)
        for (uint_t ky = 0; ky < kernel.dims[1]; ++ky) {
            const auto kw = kernel.safe(kx, ky);
            if (kw == 0.0) continue;

            uint_t x0 = (kx >= hxsize ? 0 : hxsize-kx);
            uint_t xn = map.dims[0] - (kx >= hxsize ? kx-hxsize : 0);
            uint_t y0 = (ky >= hysize ? 0 : hysize-ky);
            uint_t yn = map.dims[1] - (ky >= hysize ? ky-hysize : 0);

            for (uint_t x = x0; x < xn; ++x)
            for (uint_t y = y0; y < yn; ++y) {
                r.safe(x+kx-hxsize,y+ky-hysize) += map.safe(x,y)*kw;
            }
        }

        return r;
    }

    // Perform the convolution of two 2D arrays, assuming the second one is the kernel.
    // Note: If the FFTW library is not used, falls back to convolve2d_naive().
    template<typename TypeY1, typename TypeY2>
    auto convolve2d(const vec<2,TypeY1>& map, const vec<2,TypeY2>& kernel) ->
        vec<2,decltype(map[0]*kernel[0])> {
    #ifdef NO_FFTW
        return convolve2d_naive(map, kernel);
    #else
        phypp_check(kernel.dims[0]%2 == 1 && kernel.dims[1]%2 == 1,
            "kernel must have odd dimensions (", kernel.dims, ")");

        uint_t hsx = kernel.dims[0]/2, hsy = kernel.dims[1]/2;

        // Pad image to prevent issues with cyclic borders
        vec2d tmap = enlarge(map, {{hsx, hsy, hsx, hsy}});

        // Resize kernel to map size, with kernel center at (0,0)
        vec2d tkernel(tmap.dims);

        vec1u px1 = hsx + uindgen(hsx);
        vec1u py1 = hsy + uindgen(hsy);
        vec1u px2 = hsx - 1 - uindgen(hsx);
        vec1u py2 = hsy - 1 - uindgen(hsy);

        vec1u ix1 = uindgen(hsx);
        vec1u iy1 = uindgen(hsy);
        vec1u ix2 = tmap.dims[0] - 1 - uindgen(hsx);
        vec1u iy2 = tmap.dims[1] - 1 - uindgen(hsy);

        tkernel(ix1,iy1) = kernel(px1, py1);
        tkernel(ix2,iy1) = kernel(px2, py1);
        tkernel(ix1,iy2) = kernel(px1, py2);
        tkernel(ix2,iy2) = kernel(px2, py2);

        // Perform the convolution in Fourrier space
        auto cimg = fft(tmap)*fft(tkernel);

        // Go back to real space and shrink map back to original dimensions
        return shrink(ifft(cimg), {{hsx, hsy, hsx, hsy}})/cimg.size();
    #endif
    }

    // Perform the convolution of two 2D arrays, assuming the second one is the kernel.
    // Note: If the FFTW library is not used, falls back to convolve2d_naive().
    template<typename T = void>
    vec2d make_transition_kernel(const vec2d& from, const vec2d& to) {
    #ifdef NO_FFTW
        static_assert(!std::is_same<T,T>::value, "this function needs the FFTW library to work");
        return vec2d();
    #else
        vec2cd cfrom = fft(from);
        vec2cd cto = fft(to);
        cto(_-(cfrom.dims[1]/2),_) /= cfrom(_-(cfrom.dims[1]/2),_);
        return shift(ifft(cto), cfrom.dims[0]/2, cfrom.dims[1]/2);
    #endif
    }

    template<typename T, typename F>
    auto boxcar(const vec<2,T>& img, uint_t hsize, F&& func) ->
        vec<2,decltype(func(flatten(img)))> {

        vec<2,decltype(func(flatten(img)))> res(img.dims);
        for (uint_t x = 0; x < img.dims[0]; ++x)
        for (uint_t y = 0; y < img.dims[1]; ++y) {
            uint_t x0 = x >= hsize ? x - hsize : 0;
            uint_t x1 = x < img.dims[0]-hsize ? x + hsize : img.dims[0]-1;
            uint_t y0 = y >= hsize ? y - hsize : 0;
            uint_t y1 = y < img.dims[1]-hsize ? y + hsize : img.dims[1]-1;

            vec<1,meta::rtype_t<T>> tmp;
            tmp.reserve((x1-x0)*(y1-y0));
            for (uint_t tx = x0; tx <= x1; ++tx)
            for (uint_t ty = y0; ty <= y1; ++ty) {
                tmp.push_back(img.safe(tx,ty));
            }

            res.safe(x,y) = func(tmp);
        }

        return res;
    }

    inline vec2b mask_inflate(vec2b m, uint_t d) {
        if (d != 0) {
            // Locate edges
            vec1u ids; ids.reserve(3*sqrt(m.size()));
            for (uint_t x : range(m.dims[0]))
            for (uint_t y : range(m.dims[1])) {
                if (!m.safe(x,y)) continue;

                if ((x > 0 && !m.safe(x-1,y)) || (x < m.dims[0]-1 && !m.safe(x+1,y)) ||
                    (y > 0 && !m.safe(x,y-1)) || (y < m.dims[1]-1 && !m.safe(x,y+1))) {
                    ids.push_back(flat_id(m, x, y));
                }
            }

            if (!ids.empty()) {
                // Make kernel
                // (will contain exactly 4*d*(d+1)/2 elements)
                vec1i x; x.reserve(2*d*(d+1));
                vec1i y; y.reserve(2*d*(d+1));

                for (int_t k : range(1, d+1))
                for (int_t l : range(4*k)) {
                    int_t ml = l%k;
                    if (l < k) {
                        x.push_back(k-ml);
                        y.push_back(-ml);
                    } else if (l < 2*k) {
                        x.push_back(-ml);
                        y.push_back(-k+ml);
                    } else if (l < 3*k) {
                        x.push_back(-k+ml);
                        y.push_back(ml);
                    } else {
                        x.push_back(ml);
                        y.push_back(k-ml);
                    }
                }

                // Apply kernel to edge points
                for (uint_t i : range(ids)) {
                    vec1u j = mult_ids(m, ids[i]);

                    for (int_t k : range(x)) {
                        int_t tx = j.safe[0] + x.safe[k];
                        int_t ty = j.safe[1] + y.safe[k];

                        if (tx >= 0 && uint_t(tx) < m.dims[0] && ty >= 0 && uint_t(ty) < m.dims[1]) {
                            m.safe(uint_t(tx), uint_t(ty)) = true;
                        }
                    }
                }
            }
        }

        return m;
    }

    // Function to segment a binary or integer map into multiple components.
    // Does no de-blending. The returned map contains IDs between 'first_id' and
    // 'first_id + nsrc' (where 'nsrc' is the number of identified segments, which is
    // provided in output). Values of 0 in the input binary map are also 0 in the
    // segmentation map.
    template <typename T, typename enable = typename std::enable_if<!std::is_pointer<T>::value>::type>
    vec2u segment(vec<2,T> map, uint_t& nsrc, uint_t first_id = 1u) {
        vec2u smap(map.dims);

        phypp_check(first_id > 0, "first ID must be > 0");

        nsrc = 0;
        uint_t id = first_id;

        std::vector<uint_t> oy, ox;
        for (uint_t y : range(map.dims[0]))
        for (uint_t x : range(map.dims[1])) {
            if (map.safe(y,x) == 0) continue;

            // Found a guy
            // Use an A* - like algorithm to navigate around and
            // figure out its extents
            oy.clear(); ox.clear();

            // This function sets a point as belonging to the current segment
            // and adds the neighboring points to a search list for inspection
            auto process_point = [&ox,&oy,&map,&smap,id](uint_t ty, uint_t tx) {
                map.safe(ty,tx) = 0;
                smap.safe(ty,tx) = id;

                auto check_add = [&ox,&oy,&map](uint_t tty, uint_t ttx) {
                    if (map.safe(tty,ttx) != 0) {
                        oy.push_back(tty);
                        ox.push_back(ttx);
                    }
                };

                if (ty != 0)             check_add(ty-1,tx);
                if (ty != map.dims[0]-1) check_add(ty+1,tx);
                if (tx != 0)             check_add(ty,tx-1);
                if (tx != map.dims[1]-1) check_add(ty,tx+1);
            };

            // Add the first point to the segment
            process_point(y, x);

            // While there are points to inspect, process them as well
            while (!ox.empty()) {
                uint_t ty = oy.back(); oy.pop_back();
                uint_t tx = ox.back(); ox.pop_back();
                process_point(ty, tx);
            }

            ++id;
        }

        nsrc = id - first_id;

        return smap;
    }

    template <typename T, typename enable = typename std::enable_if<!std::is_pointer<T>::value>::type>
    vec2u segment(vec<2,T> map) {
        uint_t nsrc = 0;
        return segment(std::move(map), nsrc);
    }
}
}

#endif
