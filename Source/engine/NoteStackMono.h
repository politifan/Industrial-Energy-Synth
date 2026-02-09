#pragma once

#include <cstdint>

namespace ies::engine
{
struct NoteStackMono final
{
    static constexpr int maxNotes = 32;

    void clear() noexcept
    {
        size = 0;
    }

    bool empty() const noexcept { return size == 0; }

    int current() const noexcept
    {
        return size > 0 ? notes[size - 1] : -1;
    }

    void noteOn (int note) noexcept
    {
        remove (note);

        if (size >= maxNotes)
        {
            // Drop the oldest note to keep the most recent ones.
            for (int i = 1; i < size; ++i)
                notes[i - 1] = notes[i];
            --size;
        }

        notes[size++] = (int8_t) note;
    }

    void noteOff (int note) noexcept
    {
        remove (note);
    }

private:
    void remove (int note) noexcept
    {
        for (int i = 0; i < size; ++i)
        {
            if ((int) notes[i] != note)
                continue;

            for (int j = i + 1; j < size; ++j)
                notes[j - 1] = notes[j];

            --size;
            return;
        }
    }

    int8_t notes[maxNotes] {};
    int size = 0;
};
} // namespace ies::engine

