interface Listing {}

interface FeaturedListing {}

function openingsLeft(listing: Listing): number {
  return 0;
}

function hire(listing: Listing): Listing {
  return listing;
}

function feature(listing: Listing, badge: string): FeaturedListing {
  return listing;
}
