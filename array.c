// Relative positions are stable (duh, that's what an array is.)
// Absolute positions are not stable. 

typedef struct array {
    void *buf;
    uint64_t num_elements_allocated;
    uint64_t num_elements_used;
    size_t bytes_per_element;
} array;

const int ARRAY_INITIAL_ELEMENTS = 10;

void array_create(array *arr, size_t bytes_per_element) {
    arr->buf = xalloc(ARRAY_INITIAL_ELEMENTS * bytes_per_element);
    arr->num_elements_allocated = ARRAY_INITIAL_ELEMENTS;
    arr->num_elements_used = 0;
    arr->bytes_per_element = bytes_per_element;
}

void *array_get(array *arr, uint64_t idx) {
    if (idx >= num_elements_used) {
        panic("Accessing array at position %lu would go out of bounds", idx);
    }

    uint8_t *element = (uint8_t *)arr->buf + (idx * arr->bytes_per_element);
    return (void *)element;
}

void array_append(array *arr, void *element) {
    if (arr->num_elements_used + 1 > arr->num_elements_allocated) {
        arr->num_elements_allocated = arr->num_elements_allocated * 2 + 1;
        void *new = xalloc(arr->num_elements_allocated * arr->bytes_per_element);
        memcpy(new, arr->buf, arr->num_elements_used * arr->bytes_per_element);
        xfree(arr->buf);
        arr->buf = new;
    }

    uint8_t *next_available_spot = (uint8_t *)arr->buf + (arr->num_elements_used * arr->bytes_per_element);
    memcpy(next_available_spot, element, arr->bytes_per_element);

    arr->num_elements_used += 1;
}

void array_free_contents(array *arr) {
    xfree(arr->buf);
    arr->buf = NULL;
    arr->num_elements_allocated = 0;
    arr->num_elements_used = 0;
}
