/*  This file is part of the KDE project
    Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
    Copyright (c) 2016 L. E. Segovia <leo.segovia@siggraph.org>
    Copyright (c) 2018 Michael Zhou <simerixh@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 */

#ifndef KISSWATCH_H
#define KISSWATCH_H

#include <KoColorSetEntry.h>
#include <QString>

#include <kritapigment_export.h>

class KRITAPIGMENT_EXPORT KisSwatch : public KoColorSetEntry
{
public:
    KisSwatch() : KoColorSetEntry() { }
    KisSwatch(const KoColor &color, const QString &name = QString())
        : KoColorSetEntry(color, name)
    { }
    virtual ~KisSwatch() { }
};

#endif // KISSWATCH_H