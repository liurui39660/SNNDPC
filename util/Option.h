#pragma once

namespace SNNDPC {
struct Option {
	/**
	 * If true, a copy of the original data will be used.
	 *
	 * If false, the original input data will be used, changed, and deleted on destruction.
	 * */
	bool shallCopyInput = true;
	/**
	 * If true, assignment will use ordinals like 0, 1, 2, ...
	 *
	 * If false, will use indices of centroids.
	 * */
	bool useOrdinalAssignment = true;
	/**
	 * If true, will normalize the data in-place.
	 *
	 * If false, nothing will happen.
	 * */
	bool shallNormalize = true;
	/**
	 * If true, fields using dynamic memory will release after the last usage.
	 *
	 * If false, they will release on destruction.
	 *
	 * Note: Those fields are allocated on the first usage.
	 * */
	bool useEarlyRelease = true;
};
}
