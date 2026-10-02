const barista: Listing = { id: 1, title: "Barista", company: "Bean There" };
const chef: Listing = { id: 2, title: "Chef", company: "Pan Am", salary: 52000, openings: 3 };
const waiter: Listing = { id: 3, title: "Waiter", company: "Pan Am", openings: 0 };

const asListing: Listing = feature(chef, "Hot");

type listingCases = [
  Expect<Equal<Listing["salary"], number | undefined>>,
  Expect<Equal<Listing["openings"], number | undefined>>,
  Expect<Equal<FeaturedListing["badge"], string>>,
  Expect<Equal<FeaturedListing["company"], string>>,
  Expect<Equal<FeaturedListing["salary"], number | undefined>>,
];

// @ts-expect-error — company is required
const nameless: Listing = { id: 9, title: "Cleaner" };

// @ts-expect-error — a featured listing needs a badge
const badgeless: FeaturedListing = { id: 9, title: "Cleaner", company: "Shiny" };

test("missing openings means one", () => {
  expect(openingsLeft(barista)).toBe(1);
});

test("openings are read as given, including 0", () => {
  expect(openingsLeft(chef)).toBe(3);
  expect(openingsLeft(waiter)).toBe(0);
});

test("hire counts down and keeps the other fields", () => {
  expect(hire(chef)).toEqual({ id: 2, title: "Chef", company: "Pan Am", salary: 52000, openings: 2 });
  expect(hire(barista)).toEqual({ id: 1, title: "Barista", company: "Bean There", openings: 0 });
});

test("hire never goes below zero", () => {
  expect(hire(waiter).openings).toBe(0);
});

test("hire does not change the original", () => {
  const listing: Listing = { id: 4, title: "Cook", company: "Pan Am", openings: 2 };
  hire(listing);
  expect(listing.openings).toBe(2);
});

test("feature adds the badge and keeps everything else", () => {
  expect(feature(chef, "Hot")).toEqual({ id: 2, title: "Chef", company: "Pan Am", salary: 52000, openings: 3, badge: "Hot" });
  expect(feature(barista, "New")).toEqual({ id: 1, title: "Barista", company: "Bean There", badge: "New" });
});

test("feature does not change the original", () => {
  const listing: Listing = { id: 5, title: "Host", company: "Pan Am" };
  const featured = feature(listing, "Urgent");
  expect(featured.badge).toBe("Urgent");
  expect(listing).toEqual({ id: 5, title: "Host", company: "Pan Am" });
});
