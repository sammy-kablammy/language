// Absolute positions are not stable. Do not hold a reference to an array
// element between resizes.

typedef struct array {
    void *buf;
    uint64_t capacity;
    uint64_t length;
    size_t bytes_per_element;
} array;

const int ARRAY_INITIAL_ELEMENTS = 10;

void array_init(array *arr, size_t bytes_per_element) {
    arr->buf = xalloc(ARRAY_INITIAL_ELEMENTS * bytes_per_element);
    arr->capacity = ARRAY_INITIAL_ELEMENTS;
    arr->length = 0;
    arr->bytes_per_element = bytes_per_element;
}

void *array_get(array *arr, uint64_t idx) {
    if (idx > arr->length) {
        panic("Accessing array at position %lu would go out of bounds", idx);
    }

    uint8_t *element = (uint8_t *)arr->buf + (idx * arr->bytes_per_element);
    return (void *)element;
}

void array_append(array *arr, void *element) {
    if (arr->length + 1 > arr->capacity) {
        arr->capacity = arr->capacity * 2 + 1;
        void *new = xalloc(arr->capacity * arr->bytes_per_element);
        memcpy(new, arr->buf, arr->length * arr->bytes_per_element);
        xfree(arr->buf);
        arr->buf = new;
    }

    uint8_t *next_available_spot = (uint8_t *)arr->buf + (arr->length * arr->bytes_per_element);
    memcpy(next_available_spot, element, arr->bytes_per_element);

    arr->length += 1;
}

void array_free_contents(array *arr) {
    xfree(arr->buf);
    arr->buf = NULL;
    arr->capacity = 0;
    arr->length = 0;
}
