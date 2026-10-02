interface Listing {
  id: number;
  title: string;
  company: string;
  salary?: number;
  openings?: number;
}

interface FeaturedListing extends Listing {
  badge: string;
}

function openingsLeft(listing: Listing): number {
  return listing.openings ?? 1;
}

function hire(listing: Listing): Listing {
  return { ...listing, openings: Math.max(0, openingsLeft(listing) - 1) };
}

function feature(listing: Listing, badge: string): FeaturedListing {
  return { ...listing, badge };
}
