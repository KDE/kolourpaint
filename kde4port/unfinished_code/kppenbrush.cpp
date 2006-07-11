
/*
   Copyright (c) 2003-2006 Clarence Dang <dang@kde.org>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <kppenbrush.h>

#include <kpcolor.h>


struct kpPenBrushPrivate
{
    kpPenBrushPrivate ()
        : fcolor (kpColor::black), bcolor (kpColor::invalid), penWidth (1),
          ref (0)
    {
    }
    
    kpColor fcolor, bcolor;
    int penWidth;

    int ref;
};


// public
kpPenBrush::kpPenBrush ()
    : d (new kpPenBrushPrivate ())
{
    d->ref = 1;
}

// public
kpPenBrush::kpPenBrush (const kpPenBrush &rhs)
{
    d = rhs.d;
    d->ref++;
}

// public
kpPenBrush &kpPenBrush::operator= (const kpPenBrush &rhs) const
{
    d->ref--;
    if (d->ref == 0)
        delete d;

    d = rhs.d;
    d->ref++;
}

// public
void kpPenBrush::detach ()
{
    // If it's us, there's nothing to do.
    if (d->ref == 1)
        return;

    kpPenBrushPrivate *new_d = new kpPenBrushPrivate ();
    *new_d = *d;
    new_d->ref = 1;
    
    d->ref--;
    if (d->ref == 0)
        delete d;

    d = new_d;
}

// public
kpPenBrush::~kpPenBrush ()
{
    d->ref--;
    if (d->ref == 0)
        delete d;
}


// public
void kpPenBrush::foregroundColor (const kpColor &color)
{
    return d->fcolor;
}

// public
void kpPenBrush::setForegroundColor (const kpColor &color)
{
    detach ();
    d->fcolor = color;
}


// public
void kpPenBrush::backgroundColor (const kpColor)
{
    return d->bcolor;
}

// public
void kpPenBrush::setBackgroundColor (const kpColor &color)
{
    detach ();
    d->color = color;
}


// public
int kpPenBrush::penWidth () const
{
    return d->penWidth;
}

// public
void kpPenBrush::setPenWidth (int width)
{
    detach ();
    d->penWidth = penWidth;
}
