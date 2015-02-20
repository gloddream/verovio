/////////////////////////////////////////////////////////////////////////////
// Name:        note.cpp
// Author:      Laurent Pugin
// Created:     2011
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#include "note.h"

//----------------------------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------------------------

#include "editorial.h"
#include "slur.h"
#include "tie.h"
#include "verse.h"
#include "vrv.h"

namespace vrv {

//----------------------------------------------------------------------------
// Note
//----------------------------------------------------------------------------

Note::Note():
	LayerElement("note-"), DurationInterface(), PitchInterface(),
    AttColoration(),
    AttNoteLogMensural(),
    AttStemmed(),
    AttTiepresent()
{
    m_drawingTieAttr = NULL;
    Reset();
}


Note::~Note()
{
    // This deletes the Tie and Slur objects if necessary
    if (m_drawingTieAttr) {
        delete m_drawingTieAttr;
    }
}
    
void Note::Reset()
{
    LayerElement::Reset();
    DurationInterface::Reset();
    PitchInterface::Reset();
    
    ResetColoration();
    ResetNoteLogMensural();
    ResetStemmed();
    ResetTiepresent();
    
    // TO BE REMOVED
    m_acciaccatura = false;
    m_embellishment = EMB_NONE;
    // tie pointers
    ResetDrawingTieAttr();
    
    m_drawingStemDir = STEMDIRECTION_NONE;
    d_stemLen = 0;
}

void Note::AddLayerElement(vrv::LayerElement *element)
{
    assert( dynamic_cast<Verse*>(element) || dynamic_cast<EditorialElement*>(element) );
    element->SetParent( this );
    m_children.push_back(element);
    Modify();
}

void Note::SetDrawingTieAttr(  )
{
    assert(!this->m_drawingTieAttr);
    if ( m_drawingTieAttr ) return;
    m_drawingTieAttr = new Tie();
    m_drawingTieAttr->SetStart( this );
}

void Note::ResetDrawingTieAttr( )
{
    if ( m_drawingTieAttr ) {
        delete m_drawingTieAttr;
        m_drawingTieAttr = NULL;
    }
}
  
int Note::GetDrawingDur( )
{
    Chord* chordParent = dynamic_cast<Chord*>(this->GetFirstParent( &typeid( Chord ), MAX_CHORD_DEPTH));
    if( chordParent )
    {
        return chordParent->GetActualDur();
    }
    else
    {
        return GetActualDur();
    }
}
    
bool Note::HasDrawingStemDir()
{
    Chord* chordParent = dynamic_cast<Chord*>(this->GetFirstParent( &typeid( Chord ), 1));
    if( chordParent )
    {
        return chordParent->HasStemDir();
    }
    else
    {
        return this->HasStemDir();
    }
}

Chord* Note::IsChordTone()
{
    return dynamic_cast<Chord*>(this->GetFirstParent( &typeid( Chord ), MAX_CHORD_DEPTH));
}
    
data_STEMDIRECTION Note::GetDrawingStemDir()
{
    Chord* chordParent = dynamic_cast<Chord*>(this->GetFirstParent( &typeid( Chord ), 1));
    if( chordParent )
    {
        return chordParent->GetStemDir();
    }
    else
    {
        return this->GetStemDir();
    }
}
    
//----------------------------------------------------------------------------
// Functors methods
//----------------------------------------------------------------------------

int Note::PrepareTieAttr( ArrayPtrVoid params )
{
    // param 0: std::vector<Note*>* that holds the current notes with open ties
    // param 1: Chord** currentChord for the current chord if in a chord
    std::vector<Note*> *currentNotes = static_cast<std::vector<Note*>*>(params[0]);
    Chord **currentChord = static_cast<Chord**>(params[1]);
    
    AttTiepresent *check = this;
    if ((*currentChord)) {
        check = (*currentChord);
    }
    assert(check);
    
    std::vector<Note*>::iterator iter = currentNotes->begin();
    while ( iter != currentNotes->end()) {
        // same octave and same pitch - this is the one!
        if ((this->GetOct()==(*iter)->GetOct()) && (this->GetPname()==(*iter)->GetPname())) {
            // right flag
            if ((check->GetTie()==TIE_m) || (check->GetTie()==TIE_t)) {
                assert( (*iter)->GetDrawingTieAttr() );
                (*iter)->GetDrawingTieAttr()->SetEnd(this);
            }
            else {
                LogWarning("Expected @tie median or terminal in note '%s', skipping it", this->GetUuid().c_str());
                (*iter)->ResetDrawingTieAttr();
            }
            iter = currentNotes->erase( iter );
            // we are done for this note
            break;
        }
        iter++;
    }

    if ((check->GetTie()==TIE_m) || (check->GetTie()==TIE_i)) {
        this->SetDrawingTieAttr();
        currentNotes->push_back(this);
    }
    
    return FUNCTOR_CONTINUE;
}
    

int Note::FillStaffCurrentTimeSpanning( ArrayPtrVoid params )
{
    // Pass it to the pseudo functor of the interface
    if (this->m_drawingTieAttr) {
        return this->m_drawingTieAttr->FillStaffCurrentTimeSpanning(params);
    }
    return FUNCTOR_CONTINUE;
}
    
int Note::PrepareLyrics( ArrayPtrVoid params )
{
    // param 0: the current Syl (unused)
    // param 1: the last Note
    // param 2: the last but one Note
    Note **lastNote = static_cast<Note**>(params[1]);
    Note **lastButOneNote = static_cast<Note**>(params[2]);
    
    (*lastButOneNote) = (*lastNote);
    (*lastNote) = this;
    
    return FUNCTOR_CONTINUE;
}
    
int Note::ResetDarwing( ArrayPtrVoid params )
{
    this->ResetDrawingTieAttr();
    return FUNCTOR_CONTINUE;
};

} // namespace vrv