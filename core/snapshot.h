#pragma once

#include "core.h"
#include "object_data.h"
#include <map>
#include <optional>

NS_NAMESPACE_BEGIN
class SceneSynchronizerBase;

struct SimulatedObjectInfo {
	ObjectNetId net_id;
	int controlled_by_peer;

	SimulatedObjectInfo() = default;

	SimulatedObjectInfo(const ObjectNetId &p_id) :
		net_id(p_id),
		controlled_by_peer(-1) {
	}

	SimulatedObjectInfo(const ObjectNetId &p_id, int p_controlled_by_peer) :
		net_id(p_id),
		controlled_by_peer(p_controlled_by_peer) {
	}

	bool operator==(const SimulatedObjectInfo &p_other) const {
		return net_id == p_other.net_id;
	}
};

struct FrameIndexWithMeta {
	/// This is set to true only when the `frame_index` comes from the server.
	/// NOTE: This is needed to know when the `frame_index` is taken from a
	///       client generated snapshot because of a partial updated snapshot
	///       was received.
	bool is_server_validated = false;
	FrameIndex frame_index = FrameIndex::NONE;

	FrameIndexWithMeta() = default;
	FrameIndexWithMeta(const FrameIndexWithMeta &) = default;

	FrameIndexWithMeta(bool p_is_server_validated, FrameIndex p_frame_index):
		is_server_validated(p_is_server_validated),
		frame_index(p_frame_index) {
	}

	FrameIndexWithMeta(FrameIndex p_frame_index):
		frame_index(p_frame_index) {
	}
};

struct Snapshot {
	FrameIndex input_id = FrameIndex::NONE;
	std::vector<SimulatedObjectInfo> simulated_objects;
	/// The Node variables in a particular frame. The order of this vector
	/// matters because the index is the `ObjectNetId`.
	/// The variable array order also matter.
	std::vector<std::vector<std::optional<VarData>>> object_vars;

	/// The executed FrameIndex for the simulating peers.
	/// NOTE: Due to the nature of the doll simulation, when comparing the
	///       server snapshot with the client snapshot this map is never checked.
	///       This map is used by the Doll-controller's reconciliation algorithm.
	std::map<int, FrameIndexWithMeta> peers_frames_index;

	bool has_custom_data = false;

	/// Custom variable specified by the user.
	/// NOTE: The user can specify a different variable depending on the passed GroupSync.
	VarData custom_data;

public:
	operator std::string() const;

	const std::vector<std::optional<VarData>> *get_object_vars(ObjectNetId p_id) const;

	/// Copy the given snapshot.
	static Snapshot make_copy(const Snapshot &p_other);
	void copy(const Snapshot &p_other);

	static bool compare(
			const SceneSynchronizerBase &scene_synchronizer,
			const Snapshot &p_snap_A,
			const Snapshot &p_snap_B,
			const int p_skip_objects_not_controlled_by_peer,
			Snapshot *r_no_rewind_recover,
			std::vector<std::string> *r_differences_info
#ifdef NS_DEBUG_ENABLED
			,
			std::vector<ObjectNetId> *r_different_node_data);
#else
	);
#endif
};

struct RollingUpdateSnapshot final : public Snapshot {
	/// This is set to true when the server sends only parts of the changed objects.
	bool was_partially_updated = false;
	bool is_just_updated_simulated_objects = false;
	bool is_just_updated_custom_data = false;
	/// The list of the updated object vars on the last update.
	std::vector<ObjectNetId> just_updated_object_vars;
};

NS_NAMESPACE_END