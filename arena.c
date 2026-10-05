// An arena is a group of same-sized allocations with a common lifetime. They
// are allocated one by one, then freed together.

// Allocations are described by ID instead of by pointer. (This is because
// resizing an arena's underlying buffer will invalidate old pointers.) To store
// and look up an allocation for later, you must translate its ID to its current
// address using arena_get_element().

// (Storing a pointer is technically possible so long as no new elements are
// allocated between storage and usage. But even a single allocation could cause
// resizing, so don't rely on this.)

// To use this arena "library" concretely:
// - create an arena
// - then initialize it with arena_init()
// - for each remaining arena_* function, write a wrapper that uses your desired
// data type

////////////////////////////////////////////////////////////////////////////////

const size_t ARENA_INITIAL_NUM_ELEMENTS = 10; // pulled this number out of my ass

typedef struct arena {
    void *buf;
    size_t bytes_per_element;
    uint32_t current_element;
    uint32_t element_capacity;
} arena;

void arena_init(arena *a, size_t bytes_per_element) {
    a->buf = xalloc(ARENA_INITIAL_NUM_ELEMENTS * bytes_per_element);
    a->bytes_per_element = bytes_per_element;
    a->current_element = 0;
    a->element_capacity = ARENA_INITIAL_NUM_ELEMENTS;
}

void *arena_get_element(arena *a, uint32_t id) {
    if (id >= a->current_element) {
        panic("Arena element ID (%u) out of bounds (expected less than %u)", id, a->current_element);
    }
    if (id + 1 > UINT32_MAX) {
        panic("this arena has used up every possible uint32 ID");
    }

    // (must cast u8* because void* pointer arithmetic is undefined behavior)
    uint8_t *element = (uint8_t *)a->buf + (id * a->bytes_per_element);
    return (void *)element;
}

uint32_t arena_alloc(arena *a) {
    if (a->current_element + 1 > a->element_capacity) {
        void *new = xalloc(2 * a->element_capacity * a->bytes_per_element);
        memcpy(new, a->buf, a->element_capacity * a->bytes_per_element);
        a->element_capacity = 2 * a->element_capacity;
        xfree(a->buf);
        a->buf = new;
    }

    return a->current_element++;
}

void arena_free_all(arena *a) {
    xfree(a->buf);
    a->buf = NULL;
    a->current_element = 0;
    a->element_capacity = 0;
}
