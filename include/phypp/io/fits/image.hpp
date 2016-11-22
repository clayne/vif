#ifndef PHYPP_IO_FITS_IMAGE_HPP
#define PHYPP_IO_FITS_IMAGE_HPP

#include "phypp/io/fits/base.hpp"

namespace phypp {
namespace fits {
    // FITS input table (read only)
    class input_image : public virtual impl::fits_impl::file_base {
    public :
        explicit input_image(const std::string& filename) :
            impl::fits_impl::file_base(impl::fits_impl::image_file, filename, impl::fits_impl::read_only) {}
        explicit input_image(const std::string& filename, uint_t hdu) :
            impl::fits_impl::file_base(impl::fits_impl::image_file, filename, impl::fits_impl::read_only) {
            reach_hdu(hdu);
        }

        input_image(input_image&&) = default;
        input_image(const input_image&) = delete;
        input_image& operator = (input_image&&) = delete;
        input_image& operator = (const input_image&&) = delete;

        template<std::size_t Dim, typename Type>
        void read(vec<Dim,Type>& v) const {
            int naxis;
            fits_get_img_dim(fptr_, &naxis, &status_);
            fits::phypp_check_cfitsio(status_, "could not read dimensions of HDU");
            phypp_check_fits(naxis == Dim, "FITS file has wrong number of dimensions "
                "(expected "+strn(Dim)+", got "+strn(naxis)+")");

            int bitpix;
            std::vector<long> naxes(naxis);
            fits_get_img_param(fptr_, naxis, &bitpix, &naxis, naxes.data(), &status_);
            fits::phypp_check_cfitsio(status_, "could not read image parameters of HDU");

            int type = impl::fits_impl::bitpix_to_type(bitpix);
            phypp_check_fits(impl::fits_impl::traits<Type>::is_convertible(type), "wrong image type "
                "(expected "+pretty_type_t(Type)+", got "+impl::fits_impl::type_to_string_(type)+")");

            type = impl::fits_impl::traits<Type>::ttype;

            for (uint_t i : range(naxis)) {
                v.dims[i] = naxes[naxis-1-i];
            }

            v.resize();

            Type def = impl::fits_impl::traits<Type>::def();
            int anynul;
            fits_read_img(fptr_, type, 1, v.size(), &def, v.data.data(), &anynul, &status_);
            fits::phypp_check_cfitsio(status_, "could not read image from HDU");
        }

        template<typename Type = double>
        Type read_pixel(vec1u p) const {
            int naxis;
            fits_get_img_dim(fptr_, &naxis, &status_);
            fits::phypp_check_cfitsio(status_, "could not read dimensions of HDU");
            phypp_check_fits(uint_t(naxis) == p.size(), "FITS file has wrong number of dimensions "
                "(expected "+strn(p.size())+", got "+strn(naxis)+")");

            int bitpix;
            std::vector<long> naxes(naxis);
            fits_get_img_param(fptr_, naxis, &bitpix, &naxis, naxes.data(), &status_);
            fits::phypp_check_cfitsio(status_, "could not read image parameters of HDU");

            uint_t ppos = 1;
            uint_t pitch = 1;
            bool no_error = true;
            for (uint_t i : range(p)) {
                no_error = no_error && p.safe[naxis-1-i] < uint_t(naxes[i]);
                ppos += p.safe[naxis-1-i]*pitch;
                pitch *= naxes[i];
            }

            if (!no_error) {
                vec<1,long> nn; nn.data = naxes; nn.dims = p.dims;
                nn = reverse(nn);
                phypp_check_fits(no_error,
                    "FITS file has too small dimensions (reading pixel "+strn(p)+
                    " in dims "+strn(nn)+")");
            }

            int type = impl::fits_impl::bitpix_to_type(bitpix);
            phypp_check_fits(impl::fits_impl::traits<Type>::is_convertible(type), "wrong image type "
                "(expected "+pretty_type_t(Type)+", got "+impl::fits_impl::type_to_string_(type)+")");

            type = impl::fits_impl::traits<Type>::ttype;

            Type val;
            Type def = impl::fits_impl::traits<Type>::def();
            int anynul;
            fits_read_img(fptr_, type, ppos, 1, &def, &val, &anynul, &status_);
            fits::phypp_check_cfitsio(status_, "could not pixel from HDU");

            return val;
        }
    };

    // Output FITS table (write only, overwrites existing files)
    class output_image : public impl::fits_impl::output_file_base {
    protected :

        explicit output_image(const std::string& filename, impl::fits_impl::readwrite_tag_t) :
            impl::fits_impl::output_file_base(impl::fits_impl::image_file, filename, impl::fits_impl::write_only),
            impl::fits_impl::file_base(impl::fits_impl::image_file, filename, impl::fits_impl::write_only) {}

    public :

        explicit output_image(const std::string& filename) :
            impl::fits_impl::output_file_base(impl::fits_impl::image_file, filename, impl::fits_impl::write_only),
            impl::fits_impl::file_base(impl::fits_impl::image_file, filename, impl::fits_impl::write_only) {}

        output_image(output_image&&) = default;
        output_image(const output_image&) = delete;
        output_image& operator = (output_image&&) = delete;
        output_image& operator = (const output_image&&) = delete;

    protected :

        template<std::size_t Dim, typename Type>
        void write_impl_(const vec<Dim,Type>& v) {
            fits_write_img(fptr_, impl::fits_impl::traits<Type>::ttype, 1, v.size(),
                const_cast<typename vec<Dim,Type>::dtype*>(v.data.data()), &status_);
            fits::phypp_check_cfitsio(status_, "could not write image to HDU");
        }

    public :

        template<std::size_t Dim, typename Type>
        void write(const vec<Dim,Type>& v) {
            std::array<long,Dim> naxes;
            for (uint_t i : range(Dim)) {
                naxes[i] = v.dims[Dim-1-i];
            }

            if (hdu_count() > 0) {
                // Check current HDU is empty or null
                auto type = hdu_type();
                phypp_check(type == fits::null_hdu || type == fits::empty_hdu,
                    "cannot write image, there is already data in this HDU");

                // Then resize it
                fits_resize_img(fptr_, impl::fits_impl::traits<meta::rtype_t<Type>>::image_type, Dim,
                    naxes.data(), &status_);
                fits::phypp_check_cfitsio(status_, "could not create image HDU");
            } else {
                // No HDU yet, just create the image in the primary array
                fits_insert_img(fptr_, impl::fits_impl::traits<meta::rtype_t<Type>>::image_type, Dim,
                    naxes.data(), &status_);
                fits::phypp_check_cfitsio(status_, "could not create image HDU");
            }

            // Finally write the data
            write_impl_(v.concretise());
        }

        void write_empty() {
            long naxes = 0;

            if (hdu_count() > 0) {
                // Check current HDU is empty or null
                auto type = hdu_type();
                phypp_check(type == fits::null_hdu || type == fits::empty_hdu,
                    "cannot write image, there is already data in this HDU");

                // Then resize it
                fits_resize_img(fptr_, impl::fits_impl::traits<float>::image_type, 0, &naxes, &status_);
                fits::phypp_check_cfitsio(status_, "could not create image HDU");
            } else {
                // And create the new HDU
                fits_insert_img(fptr_, impl::fits_impl::traits<float>::image_type, 0, &naxes, &status_);
                fits::phypp_check_cfitsio(status_, "could not create empty image HDU");
            }

            // Update internal structures, because CFITSIO won't do that by itself
            fits_set_hdustruc(fptr_, &status_);
            fits::phypp_check_cfitsio(status_, "could not create empty image HDU");
        }
    };

    // Input/output FITS table (read & write, modifies existing files)
    class image : public output_image, public input_image {
    public :
        explicit image(const std::string& filename) :
            impl::fits_impl::file_base(impl::fits_impl::image_file, filename, impl::fits_impl::read_write),
            output_image(filename, impl::fits_impl::readwrite_tag), input_image(filename) {}

        explicit image(const std::string& filename, uint_t hdu) :
            impl::fits_impl::file_base(impl::fits_impl::image_file, filename, impl::fits_impl::read_write),
            output_image(filename, impl::fits_impl::readwrite_tag), input_image(filename) {
            reach_hdu(hdu);
        }

        image(image&&) = default;
        image(const image&) = delete;
        image& operator = (image&&) = delete;
        image& operator = (const image&&) = delete;

        template<std::size_t Dim, typename Type>
        void update(const vec<Dim,Type>& v) {
            int naxis;
            fits_get_img_dim(fptr_, &naxis, &status_);
            fits::phypp_check_cfitsio(status_, "could not read dimensions of HDU");
            phypp_check_fits(naxis == Dim, "FITS file has wrong number of dimensions "
                "(expected "+strn(Dim)+", got "+strn(naxis)+")");

            int bitpix;
            std::vector<long> naxes(naxis);
            fits_get_img_param(fptr_, naxis, &bitpix, &naxis, naxes.data(), &status_);
            fits::phypp_check_cfitsio(status_, "could not read image parameters of HDU");

            int type = impl::fits_impl::bitpix_to_type(bitpix);
            phypp_check_fits(impl::fits_impl::traits<meta::rtype_t<Type>>::is_convertible(type), "wrong image type "
                "(expected "+pretty_type_t(meta::rtype_t<Type>)+", got "+impl::fits_impl::type_to_string_(type)+")");

            std::array<uint_t,Dim> d;
            for (uint_t i : range(Dim)) {
                d[i] = naxes[Dim-1-i];
            }

            phypp_check_fits(v.dims == d, "incompatible array dimensions ("+strn(v.dims)+
                " vs. "+strn(d)+")");

            write_impl_(v.concretise());
        }
    };
}
}

#endif
