/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_RECORDED_ACTION_H_
#define _KIS_RECORDED_ACTION_H_

class QDomDocument;
class QDomElement;
class QString;

#include <krita_export.h>

class KRITAIMAGE_EXPORT KisRecordedAction {
    struct Private;
    public:
        KisRecordedAction(QString name, QString id);
        virtual ~KisRecordedAction();
        virtual void play() = 0;
        virtual void toXML(QDomDocument& doc, QDomElement elt);
        virtual void fromXML(QDomElement elt);
    public:
        QString id();
        QString name();
    private:
        Private* const d;
};

#endif
