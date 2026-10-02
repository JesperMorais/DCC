const hd = parseSize("1920x1080");
const thumb = fitInside([4000, 3000], [800, 800]);

type cases = [
  Expect<Equal<Size, [number, number]>>,
  Expect<Equal<typeof hd, Size | null>>,
  Expect<Equal<typeof thumb, Size>>,
];

// @ts-expect-error — a Size has exactly two numbers
const tooLong: Size = [1, 2, 3];

// @ts-expect-error — width and height are numbers, not strings
const wrongType: Size = ["800", "600"];

test("parseSize reads width and height", () => {
  expect(hd).toEqual([1920, 1080]);
});

test("parseSize allows spaces around the numbers", () => {
  expect(parseSize(" 800 x 600 ")).toEqual([800, 600]);
});

test("parseSize rejects zero, decimals and non-numbers", () => {
  expect(parseSize("0x10")).toBeNull();
  expect(parseSize("12.5x3")).toBeNull();
  expect(parseSize("x600")).toBeNull();
  expect(parseSize("abcx600")).toBeNull();
  expect(parseSize("")).toBeNull();
});

test("parseSize needs exactly one x", () => {
  expect(parseSize("1x2x3")).toBeNull();
  expect(parseSize("1920")).toBeNull();
});

test("fitInside scales landscape and portrait to the box", () => {
  expect(thumb).toEqual([800, 600]);
  expect(fitInside([1000, 2000], [500, 500])).toEqual([250, 500]);
});

test("fitInside rounds to whole pixels", () => {
  expect(fitInside([1000, 333], [500, 500])).toEqual([500, 167]);
});

test("fitInside never scales up", () => {
  expect(fitInside([300, 200], [800, 800])).toEqual([300, 200]);
});
