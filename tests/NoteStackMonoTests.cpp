#include <cassert>

#include "../Source/engine/NoteStackMono.h"

using ies::engine::NoteStackMono;

static void test_empty()
{
    NoteStackMono s;
    assert(s.empty());
    assert(s.current() == -1);
}

static void test_last_note_priority()
{
    NoteStackMono s;
    s.noteOn(60); // C4
    assert(!s.empty());
    assert(s.current() == 60);

    s.noteOn(64); // E4
    assert(s.current() == 64);

    s.noteOff(64);
    assert(s.current() == 60);
}

static void test_duplicate_note_on_moves_to_end()
{
    NoteStackMono s;
    s.noteOn(60);
    s.noteOn(64);
    s.noteOn(67);
    assert(s.current() == 67);

    // Press 64 again: it should become current.
    s.noteOn(64);
    assert(s.current() == 64);

    s.noteOff(64);
    assert(s.current() == 67);
}

static void test_note_off_nonexistent_is_noop()
{
    NoteStackMono s;
    s.noteOn(60);
    s.noteOn(64);
    s.noteOff(67);
    assert(s.current() == 64);
}

static void test_capacity_drops_oldest()
{
    NoteStackMono s;
    for (int i = 0; i < NoteStackMono::maxNotes; ++i)
        s.noteOn(i);
    assert(s.current() == NoteStackMono::maxNotes - 1);

    // Push one more: oldest (0) is dropped; current is the new note.
    s.noteOn(127);
    assert(s.current() == 127);

    // Remove current, should fall back to previous last (maxNotes-1).
    s.noteOff(127);
    assert(s.current() == NoteStackMono::maxNotes - 1);
}

int main()
{
    test_empty();
    test_last_note_priority();
    test_duplicate_note_on_moves_to_end();
    test_note_off_nonexistent_is_noop();
    test_capacity_drops_oldest();
    return 0;
}

