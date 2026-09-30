function largest(nums: number[]): number {
  let best = nums[0];
  for (const n of nums) {
    if (n > best) {
      best = n;
    }
  }
  return best;
}
