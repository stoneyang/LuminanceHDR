/*
 * This file is a part of Luminance HDR package.
 * ----------------------------------------------------------------------
 * Copyright (C) 2012 Franco Comida
 * Copyright (C) 2012-2013 Davide Anastasia
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ----------------------------------------------------------------------
 */

//! \author Franco Comida <fcomida@users.sourceforge.net>
//! Original work
//! \author Davide Anastasia <davideanastasia@users.sourceforge.net>
//! - clean up memory management
//! - rewrite for LibHDR
//! - implementation in-memory writer (to retrieve size)

#ifndef JPEGWRITER_H
#define JPEGWRITER_H

#include <string>
#include <boost/scoped_ptr.hpp>
#include <Libpfs/params.h>
#include <Libpfs/io/ioexception.h>

namespace pfs {
class Frame;
}

class JpegWriterImpl;

class JpegWriter
{
public:
    JpegWriter(const std::string& filename);
    JpegWriter();
    ~JpegWriter();

    //! \brief write a pfs::Frame into file or memory
    bool write(const pfs::Frame& frame, const pfs::Params& params);

    //! \brief return size in bytes of the file written
    size_t getFileSize();

private:
    boost::scoped_ptr<JpegWriterImpl> m_impl;
};

#endif