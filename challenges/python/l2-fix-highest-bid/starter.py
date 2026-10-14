def highest_bid(bids: list[str]) -> int:
    cleaned = [bid.strip() for bid in bids if bid.strip()]
    if not cleaned:
        return 0
    return int(max(cleaned))
