#ifndef PHYPP_IO_FITS_BASE_HPP
#define PHYPP_IO_FITS_BASE_HPP

#include <fitsio.h>
#include <string>
#include "phypp/core/vec.hpp"
#include "phypp/core/print.hpp"
#include "phypp/core/error.hpp"
#include "phypp/utility/generic.hpp"
#include "phypp/io/filesystem.hpp"
#include "phypp/io/ascii.hpp"

namespace phypp {
namespace fits {
    struct exception : std::exception {
        explicit exception(const std::string& m) : msg(m) {}
        std::string msg;

        const char* what() const noexcept override {
            return msg.c_str();
        }
    };

    #define phypp_check_fits(assertion, msg) \
        do { if (!(assertion)) throw phypp::fits::exception(msg); } while(0)

    inline void phypp_check_cfitsio(int status, const std::string& msg) {
        if (status != 0) {
            char txt[FLEN_STATUS];
            fits_get_errstatus(status, txt);
            char tdetails[FLEN_ERRMSG];
            std::string details;
            while (fits_read_errmsg(tdetails)) {
                if (!details.empty()) details += "\ncfitsio: ";
                details += std::string(tdetails);
            }

            phypp_check_fits(status == 0, "error: cfitsio: "+std::string(txt)+"\ncfitsio: "+
                details+"\nerror: "+msg);
        }
    }

    using header = std::string;

    enum hdu_type {
        null_hdu = 0,
        empty_hdu,
        image_hdu,
        table_hdu
    };
}

namespace impl {
    namespace fits_impl {
        template<typename T>
        struct traits;

        template<>
        struct traits<std::string> {
            using dtype = char;

            static const char tform = 'A';
            static const int ttype = TSTRING;

            static std::string def() {
                return "";
            }

            static bool is_convertible(int type) {
                if (type == TSTRING) return true;
                return false;
            }

            static bool is_convertible_narrow(int type) {
                if (type == TSTRING) return true;
                return false;
            }
        };

        template<>
        struct traits<bool> {
            using dtype = char;

            static const char tform = 'B';
            static const int ttype = TBYTE;
            static const int image_type = BYTE_IMG;

            static bool def() {
                return false;
            }

            static bool is_convertible(int type) {
                if (type == TLOGICAL) return true;
                if (type == TBIT) return true;
                if (type == TBYTE) return true;
                return false;
            }

            static bool is_convertible_narrow(int type) {
                if (type == TLOGICAL) return true;
                if (type == TBIT) return true;
                if (type == TBYTE) return true;
                if (type == TSHORT) return true;
                if (type == TLONG) return true;
                if (type == TLONGLONG) return true;
                if (type == TINT32BIT) return true;
                return false;
            }
        };

        template<>
        struct traits<char> {
            using dtype = char;

            static const char tform = 'S';
            static const int ttype = TSBYTE;
            static const int image_type = SBYTE_IMG;

            static bool def() {
                return '\0';
            }

            static bool is_convertible(int type) {
                if (type == TBYTE) return true;
                return false;
            }

            static bool is_convertible_narrow(int type) {
                if (type == TBYTE) return true;
                if (type == TSHORT) return true;
                if (type == TLONG) return true;
                if (type == TLONGLONG) return true;
                if (type == TINT32BIT) return true;
                return false;
            }
        };

        template<>
        struct traits<uint_t> {
            using dtype = uint_t;

            static const char tform = 'K';
            static const int ttype = TLONGLONG;
            static const int image_type = LONGLONG_IMG;

            static uint_t def() {
                return 0;
            }

            static bool is_convertible(int type) {
                if (type == TSHORT) return true;
                if (type == TLONG) return true;
                if (type == TLONGLONG) return true;
                if (type == TBIT) return true;
                if (type == TBYTE) return true;
                if (type == TINT32BIT) return true;
                return false;
            }

            static bool is_convertible_narrow(int type) {
                if (type == TSHORT) return true;
                if (type == TLONG) return true;
                if (type == TLONGLONG) return true;
                if (type == TBIT) return true;
                if (type == TBYTE) return true;
                if (type == TINT32BIT) return true;
                return false;
            }
        };

        template<>
        struct traits<int_t> {
            using dtype = int_t;

            static const char tform = 'K';
            static const int ttype = TLONGLONG;
            static const int image_type = LONGLONG_IMG;

            static int_t def() {
                return 0;
            }

            static bool is_convertible(int type) {
                if (type == TSHORT) return true;
                if (type == TLONG) return true;
                if (type == TLONGLONG) return true;
                if (type == TBIT) return true;
                if (type == TBYTE) return true;
                if (type == TINT32BIT) return true;
                return false;
            }

            static bool is_convertible_narrow(int type) {
                if (type == TSHORT) return true;
                if (type == TLONG) return true;
                if (type == TLONGLONG) return true;
                if (type == TBIT) return true;
                if (type == TBYTE) return true;
                if (type == TINT32BIT) return true;
                return false;
            }
        };

        template<>
        struct traits<float> {
            using dtype = float;

            static const char tform = 'E';
            static const int ttype = TFLOAT;
            static const int image_type = FLOAT_IMG;

            static float def() {
                return fnan;
            }

            static bool is_convertible(int type) {
                if (type == TSHORT) return true;
                if (type == TLONG) return true;
                if (type == TLONGLONG) return true;
                if (type == TFLOAT) return true;
                if (type == TBIT) return true;
                if (type == TBYTE) return true;
                if (type == TINT32BIT) return true;
                return false;
            }

            static bool is_convertible_narrow(int type) {
                if (type == TSHORT) return true;
                if (type == TLONG) return true;
                if (type == TLONGLONG) return true;
                if (type == TFLOAT) return true;
                if (type == TDOUBLE) return true;
                if (type == TBIT) return true;
                if (type == TBYTE) return true;
                if (type == TINT32BIT) return true;
                return false;
            }
        };

        template<>
        struct traits<double> {
            using dtype = double;

            static const char tform = 'D';
            static const int ttype = TDOUBLE;
            static const int image_type = DOUBLE_IMG;

            static float def() {
                return dnan;
            }

            static bool is_convertible(int type) {
                if (type == TSHORT) return true;
                if (type == TLONG) return true;
                if (type == TLONGLONG) return true;
                if (type == TFLOAT) return true;
                if (type == TDOUBLE) return true;
                if (type == TBIT) return true;
                if (type == TBYTE) return true;
                if (type == TINT32BIT) return true;
                return false;
            }

            static bool is_convertible_narrow(int type) {
                if (type == TSHORT) return true;
                if (type == TLONG) return true;
                if (type == TLONGLONG) return true;
                if (type == TFLOAT) return true;
                if (type == TDOUBLE) return true;
                if (type == TBIT) return true;
                if (type == TBYTE) return true;
                if (type == TINT32BIT) return true;
                return false;
            }
        };

        inline int bitpix_to_type(int bitpix) {
            switch(bitpix) {
                case BYTE_IMG     : return TBYTE;
                case SHORT_IMG    : return TSHORT;
                case LONG_IMG     : return TLONG;
                case LONGLONG_IMG : return TLONGLONG;
                case FLOAT_IMG    : return TFLOAT;
                case DOUBLE_IMG   : return TDOUBLE;
                default : throw fits::exception("unknown image type '"+strn(bitpix)+"'");
            }
        }

        inline std::string type_to_string_(int type) {
            if (type == TSTRING) return "string";
            if (type == TSHORT) return "short";
            if (type == TLONG) return "long";
            if (type == TLONGLONG) return "long long";
            if (type == TFLOAT) return "float";
            if (type == TDOUBLE) return "double";
            if (type == TLOGICAL) return "bool";
            if (type == TBIT) return "bool";
            if (type == TBYTE) return "char";
            if (type == TINT32BIT) return "int32";
            if (type == TCOMPLEX) return "complex float";
            if (type == TDBLCOMPLEX) return "complex double";
            return "unknown ("+strn(type)+")";
        }

        enum file_type {
            generic_file,
            image_file,
            table_file
        };

        enum access_right  {
            write_only,
            read_only,
            read_write
        };

        template<typename T>
        struct is_string : std::false_type {};

        template<>
        struct is_string<std::string> : std::true_type {};
        template<>
        struct is_string<char*> : std::true_type {};
        template<>
        struct is_string<const char*> : std::true_type {};
        template<std::size_t N>
        struct is_string<char[N]> : std::true_type {};
        template<std::size_t N>
        struct is_string<const char[N]> : std::true_type {};

        class file_base {
        public :
            file_base(file_type type, const std::string& filename, access_right rights) :
                type_(type), rights_(rights), filename_(filename) {

                if (rights == write_only || (rights == read_write && !file::exists(filename))) {
                    fits_create_file(&fptr_, ("!"+filename).c_str(), &status_);
                    fits::phypp_check_cfitsio(status_, "cannot create file '"+filename+"'");
                } else {
                    int trights = (rights == read_only ? READONLY : READWRITE);

                    switch (type) {
                        case generic_file :
                            fits_open_file(&fptr_, filename.c_str(), trights, &status_);
                            break;
                        case image_file :
                            fits_open_image(&fptr_, filename.c_str(), trights, &status_);
                            break;
                        case table_file :
                            fits_open_table(&fptr_, filename.c_str(), trights, &status_);
                            break;
                    }

                    fits::phypp_check_cfitsio(status_, "cannot open file '"+filename+"'");
                }
            }

            file_base(file_base&& in) noexcept : type_(in.type_), rights_(in.rights_),
                filename_(in.filename_), fptr_(in.fptr_), status_(in.status_) {
                in.fptr_ = nullptr;
            }

            void close() {
                status_ = 0;
                fits_close_file(fptr_, &status_);
                fptr_ = nullptr;
            }

            const std::string& filename() const {
                return filename_;
            }

            int cfitsio_status() const {
                return status_;
            }

            fitsfile* cfitsio_ptr() {
                return fptr_;
            }

            const fitsfile* cfitsio_ptr() const {
                return fptr_;
            }

            fits::header read_header() const {
                status_ = 0;
                fits::header hdr;
                char* hstr = nullptr;
                int nkeys  = 0;
                fits_hdr2str(fptr_, 0, nullptr, 0, &hstr, &nkeys, &status_);
                fits::phypp_check_cfitsio(status_, "could not dump header to string");
                hdr = hstr;
                fits_free_memory(hstr, &status_);
                return hdr;
            }

            bool has_keyword(const std::string& name) const {
                char comment[80];
                char content[80];
                int status = 0;
                fits_read_keyword(fptr_, name.c_str(), content, comment, &status);
                return status == 0;
            }

            bool read_keyword(const std::string& name, std::string& value) const {
                char comment[80];
                char content[80];
                int status = 0;
                fits_read_keyword(fptr_, name.c_str(), content, comment, &status);
                value = content;
                value = trim(value, " '");
                return status == 0;
            }

            template <typename T>
            bool read_keyword(const std::string& name, T& value) const {
                std::string content;
                if (!read_keyword(name, content)) return false;

                // Convert FITS boolean values to something we can read
                if (content == "T") {
                    content = "1";
                } else if (content == "F") {
                    content = "0";
                }

                return from_string(content, value);
            }

            void write_header(const fits::header& hdr) {
                std::size_t nentry = hdr.size()/80 + 1;
                fits_set_hdrsize(fptr_, nentry, &status_);

                for (uint_t i : range(nentry)) {
                    std::string entry = hdr.substr(i*80, std::min(std::size_t(80), hdr.size() - i*80));
                    if (start_with(entry, "END ")) continue;

                    // Skip if it is an internal FITS keyword
                    std::size_t eqpos = entry.find_first_of("=");
                    if (eqpos != entry.npos) {
                        std::string nam = trim(entry.substr(0, eqpos));
                        if (nam == "SIMPLE" || nam == "BITPIX" || start_with(nam, "NAXIS") ||
                            nam == "EXTEND" || nam == "XTENSION" || nam == "EXTNAME" ||
                            nam == "PCOUNT" || nam == "GCOUNT") {
                            continue;
                        }
                    }

                    fits_write_record(fptr_, entry.c_str(), &status_);
                    fits::phypp_check_cfitsio(status_, "could not write header");
                }
            }

            template<typename T, typename enable = typename
                std::enable_if<!is_string<meta::decay_t<T>>::value>::type>
            void write_keyword(const std::string& name, const T& value,
                const std::string& com = "") {
                fits_update_key(fptr_, traits<meta::decay_t<T>>::ttype,
                    name.c_str(), const_cast<T*>(&value),
                    const_cast<char*>(com.c_str()), &status_);
                fits::phypp_check_cfitsio(status_, "could not write keyword '"+name+"'");
            }

            template<typename T, typename enable = typename
                std::enable_if<!is_string<meta::decay_t<T>>::value>::type>
            void add_keyword(const std::string& name, const T& value,
                const std::string& com = "") {
                fits_write_key(fptr_, traits<meta::decay_t<T>>::ttype,
                    name.c_str(), const_cast<T*>(&value),
                    const_cast<char*>(com.c_str()), &status_);
                fits::phypp_check_cfitsio(status_, "could not write keyword '"+name+"'");
            }

            void write_keyword(const std::string& name, const std::string& value,
                const std::string& com = "") {
                fits_update_key(fptr_, TSTRING, name.c_str(),
                    const_cast<char*>(value.c_str()),
                    const_cast<char*>(com.c_str()), &status_);
                fits::phypp_check_cfitsio(status_, "could not write keyword '"+name+"'");
            }

            void add_keyword(const std::string& name, const std::string& value,
                const std::string& com = "") {
                fits_write_key(fptr_, TSTRING, name.c_str(),
                    const_cast<char*>(value.c_str()),
                    const_cast<char*>(com.c_str()), &status_);
                fits::phypp_check_cfitsio(status_, "could not write keyword '"+name+"'");
            }

            void remove_keyword(const std::string& name) {
                fits_delete_key(fptr_, name.c_str(), &status_);
                status_ = 0; // don't care if the keyword doesn't exist or other errors
            }

            uint_t hdu_count() const {
                int nhdu = 0;
                fits_get_num_hdus(fptr_, &nhdu, &status_);
                fits::phypp_check_cfitsio(status_, "could not get the number of HDUs");
                return nhdu;
            }

            uint_t current_hdu() const {
                int hdu = 1;
                fits_get_hdu_num(fptr_, &hdu);
                return hdu-1;
            }

            fits::hdu_type hdu_type() const {
                bool simple = false;
                if (read_keyword("SIMPLE", simple)) {
                    if (axis_count() == 0) {
                        return fits::empty_hdu;
                    } else {
                        return fits::image_hdu;
                    }
                } else {
                    std::string xtension;
                    if (read_keyword("XTENSION", xtension)) {
                        if (xtension == "IMAGE") {
                            if (axis_count() == 0) {
                                return fits::empty_hdu;
                            } else {
                                return fits::image_hdu;
                            }
                        } else if (xtension == "BINTABLE") {
                            return fits::table_hdu;
                        } else {
                            phypp_check(false, "unknown XTENSION value '", xtension, "'");
                        }
                    }
                }

                return fits::null_hdu;
            }

            void reach_hdu(uint_t hdu) {
                uint_t nhdu = hdu_count();
                if (rights_ == read_only) {
                    phypp_check(hdu < nhdu, "requested HDU does not exists in this "
                        "FITS file (", hdu, " vs. ", nhdu, ")");
                } else {
                    if (hdu >= nhdu) {
                        // Create missing HDUs to be able to reach the one requested
                        long naxes = 0;
                        for (uint_t i : range(hdu-nhdu+1)) {
                            fits_insert_img(fptr_, impl::fits_impl::traits<float>::image_type,
                                0, &naxes, &status_);
                            fits::phypp_check_cfitsio(status_,
                                "could not create new HDUs to reach HDU "+strn(hdu));
                        }

                        fits_set_hdustruc(fptr_, &status_);
                        fits::phypp_check_cfitsio(status_,
                            "could not create new HDUs to reach HDU "+strn(hdu));
                    }
                }

                fits_movabs_hdu(fptr_, hdu+1, nullptr, &status_);
                fits::phypp_check_cfitsio(status_, "could not reach HDU "+strn(hdu));
            }

            // Return the number of dimensions of a FITS file
            // Note: will return 0 for FITS tables
            uint_t axis_count() const {
                int naxis;
                fits_get_img_dim(fptr_, &naxis, &status_);
                fits::phypp_check_cfitsio(status_, "could not get the number of axis in HDU");
                return naxis;
            }

            bool is_cube() const {
                return axis_count() == 3;
            }

            bool is_image() const {
                return axis_count() == 2;
            }

            bool is_spectrum() const {
                return axis_count() == 1;
            }

            bool is_table() const {
                return axis_count() == 0;
            }

            // Return the dimensions of a FITS image
            // Note: will return an empty array for FITS tables
            vec1u image_dims() const {
                uint_t naxis = axis_count();
                vec1u dims(naxis);
                if (naxis != 0) {
                    std::vector<long> naxes(naxis);
                    fits_get_img_size(fptr_, naxis, naxes.data(), &status_);
                    fits::phypp_check_cfitsio(status_, "could not get image size of HDU");
                    for (uint_t i : range(naxis)) {
                        dims.safe[i] = naxes[naxis-1-i];
                    }
                }

                return dims;
            }

            virtual ~file_base() {
                close();
            }

        protected :
            const file_type type_ = generic_file;
            const access_right rights_ = read_write;
            const std::string filename_;
            fitsfile* fptr_ = nullptr;
            mutable int status_ = 0;
        };

        class output_file_base : virtual public file_base {
        public :
            output_file_base(file_type type, const std::string& filename, access_right rights) :
                file_base(type, filename, rights) {}

            output_file_base(output_file_base&& in) noexcept : file_base(std::move(in)) {}

            void remove_hdu() {
                fits_delete_hdu(fptr_, nullptr, &status_);
                fits::phypp_check_cfitsio(status_, "could not remove the current HDU");
            }
        };

        static struct readwrite_tag_t {} readwrite_tag;
    }
}

namespace fits {
    // Generic FITS file (read only)
    class generic_file : public virtual impl::fits_impl::file_base {
    public :
        explicit generic_file(const std::string& filename) :
            impl::fits_impl::file_base(impl::fits_impl::generic_file, filename, impl::fits_impl::read_only) {}
        explicit generic_file(const std::string& filename, uint_t hdu) :
            impl::fits_impl::file_base(impl::fits_impl::generic_file, filename, impl::fits_impl::read_only) {
            reach_hdu(hdu);
        }

        generic_file(generic_file&&) noexcept = default;
    };

    // Header keyword manipulation
    template<typename T>
    bool getkey(const fits::header& hdr, const std::string& key, T& v) {
        std::size_t nentry = hdr.size()/80 + 1;
        for (uint_t i : range(nentry)) {
            std::string entry = hdr.substr(i*80, std::min(std::size_t(80), hdr.size() - i*80));
            std::size_t eqpos = entry.find_first_of("=");
            if (eqpos == entry.npos) continue;
            std::size_t cpos = entry.find_first_of("/", eqpos);

            std::string nam = trim(entry.substr(0, eqpos));

            if (nam == key) {
                std::string value = trim(entry.substr(eqpos+1, cpos-eqpos-1));
                return from_string(value, v);
            }
        }

        return false;
    }

    inline bool getkey(const fits::header& hdr, const std::string& key, std::string& v) {
        std::size_t nentry = hdr.size()/80 + 1;
        for (uint_t i : range(nentry)) {
            std::string entry = hdr.substr(i*80, std::min(std::size_t(80), hdr.size() - i*80));
            std::size_t eqpos = entry.find_first_of("=");
            if (eqpos == entry.npos) continue;
            std::size_t cpos = entry.find_first_of("/", eqpos);

            std::string nam = trim(entry.substr(0, eqpos));

            if (nam == key) {
                std::string value = trim(entry.substr(eqpos+1, cpos-eqpos-1));
                if (value.find_first_of("'") == 0) {
                    value = trim(value, "'");
                }
                v = value;
                return true;
            }
        }

        return false;
    }

    template<typename T>
    bool setkey(fits::header& hdr, const std::string& key, const T& v,
        const std::string& comment = "") {

        if (key.size() > 8) return false;

        // Build new entry
        std::string entry = key+std::string(8-key.size(), ' ')+"= ";
        std::string value = strn(v);
        if (!comment.empty()) {
            value += " / "+comment;
        }

        entry += value;
        if (entry.size() > 80) return false;

        // Insertion point
        std::size_t ipos = hdr.npos;
        if (hdr.size() >= 80) {
            ipos = hdr.substr(hdr.size() - 80, 80).find("END");
            if (ipos != hdr.npos) ipos += hdr.size() - 80;
        }

        // Look for existing entry in header
        std::size_t nentry = hdr.size()/80 + 1;
        for (uint_t i : range(nentry)) {
            // Extract entry 'i'
            std::string tentry = hdr.substr(i*80, std::min(std::size_t(80), hdr.size() - i*80));
            std::size_t eqpos = tentry.find_first_of("=");
            if (eqpos == tentry.npos) continue;

            // Extract entry's name
            std::string nam = trim(tentry.substr(0, eqpos));
            // Look for comments
            std::size_t cpos = tentry.find_first_of("/", eqpos);

            if (nam == key) {
                // The entry already exists
                ipos = i*80;

                // Copy comments of the previous entry
                if (comment.empty() && cpos != tentry.npos) {
                    std::string cmt = tentry.substr(cpos+1);
                    entry += " /"+cmt;
                    if (entry.size() > 80) {
                        if (entry.back() == '&') {
                            entry.resize(80);
                            entry.back() = '&';
                        } else {
                            entry.resize(80);
                        }
                    }
                }

                hdr.erase(ipos, 80);

                break;
            }
        }

        entry += std::string(80-entry.size(), ' ');
        if (ipos == hdr.npos) {
            hdr += entry;
        } else {
            hdr.insert(ipos, entry);
        }

        return true;
    }

    struct header_keyword {
        std::string key, value, comment;
        bool novalue = true;
    };

    using parsed_header = vec<1,header_keyword>;

    inline fits::parsed_header parse_header(const fits::header& hdr) {
        fits::parsed_header keys;
        vec1s ckeys = cut(hdr, 80);
        keys.resize(ckeys.size());
        for (uint_t i : range(ckeys)) {
            if (ckeys[i].find_first_of("HISTORY ") == 0) {
                keys[i].key = trim(ckeys[i]);
            } else {
                auto p = ckeys[i].find_first_of("=/");
                if (p == std::string::npos) {
                    keys[i].key = trim(ckeys[i]);
                } else if (ckeys[i][p] == '/') {
                    keys[i].key = trim(ckeys[i].substr(0, p));
                    keys[i].comment = trim(ckeys[i].substr(p));
                } else if (ckeys[i][p] == '=') {
                    keys[i].novalue = false;
                    keys[i].key = trim(ckeys[i].substr(0, p));

                    std::string right = trim(ckeys[i].substr(p+1));
                    if (!right.empty()) {
                        if (right[0] == '\'') {
                            auto p2 = right.find_first_of('\'', 1);
                            if (p2 != std::string::npos) {
                                ++p2;

                                keys[i].value = trim(right.substr(0, p2));
                                keys[i].comment = trim(right.substr(p2));
                            } else {
                                keys[i].value = trim(right);
                            }
                        } else {
                            uint_t p2 = right.find_first_of('/');
                            if (p2 != std::string::npos) {
                                keys[i].value = trim(right.substr(0, p2));
                                keys[i].comment = trim(right.substr(p2));
                            } else {
                                keys[i].value = trim(right);
                            }
                        }
                    }
                }
            }
        }

        return keys;
    }

    inline fits::header serialize_header(const fits::parsed_header& keys) {
        fits::header hdr;
        hdr.reserve(80*keys.size());
        for (auto& k : keys) {
            std::string entry;
            if (!k.novalue) {
                if (start_with(k.key, "HIERARCH ")) {
                    entry = k.key+" = "+k.value;
                } else {
                    std::string val;
                    if (k.value[0] == '\'') {
                        val = align_left(k.value, 20, ' ');
                    } else {
                        val = align_right(k.value, 20, ' ');
                    }
                    entry = align_left(k.key, 8, ' ')+"= "+val;
                }
            } else {
                entry = align_left(k.key, 30, ' ');
            }

            if (!k.comment.empty()) {
                entry += " "+k.comment;
            }

            if (entry.size() > 80) {
                entry = entry.substr(0, 80);
            } else if (entry.size() < 80) {
                entry += std::string(80-entry.size(), ' ');
            }

            hdr += entry;
        }

        return hdr;
    }
}
}

#endif
