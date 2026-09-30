from itertools import count, islice


def test_last_chunk_may_be_shorter():
    assert list(chunked([1, 2, 3, 4, 5, 6, 7], 3)) == [[1, 2, 3], [4, 5, 6], [7]]


def test_no_empty_chunk_when_it_divides_evenly():
    assert list(chunked([1, 2, 3, 4], 2)) == [[1, 2], [3, 4]]


def test_empty_input_yields_nothing():
    assert list(chunked([], 5)) == []


def test_works_on_any_iterable():
    assert list(chunked("abcde", 2)) == [["a", "b"], ["c", "d"], ["e"]]
    assert list(chunked((x * x for x in range(4)), 3)) == [[0, 1, 4], [9]]


def test_n_below_one_is_rejected():
    with raises(ValueError):
        list(chunked([1, 2, 3], 0))


def test_pulls_only_what_the_next_chunk_needs():
    pulled: list[int] = []

    def rows():
        for i in range(100):
            pulled.append(i)
            yield i

    batches = chunked(rows(), 4)
    assert pulled == []
    assert next(batches) == [0, 1, 2, 3]
    assert len(pulled) == 4
    assert next(batches) == [4, 5, 6, 7]
    assert len(pulled) == 8


def test_works_on_an_infinite_iterator():
    assert list(islice(chunked(count(), 2), 3)) == [[0, 1], [2, 3], [4, 5]]
