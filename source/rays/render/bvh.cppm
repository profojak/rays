module;

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <limits>
#include <optional>
#include <ranges>
#include <type_traits>
#include <vector>

export module rays:bvh;

import :bounds;
import :mesh;
import :ray;
import :triangle;
import :type;
import :vector;

namespace rays {

/// Two-level SBVH tree.
export class BVH {

  private:
    /// BVH node.
    struct alignas(32) Node {
        /// Bounding box.
        Bounds3f bounds;
        /// First reference index.
        UInt first{0};
        /// Second reference index.
        UInt second{0};

        /// Mask used to distinguish leaf nodes from internal nodes.
        static constexpr UInt leafMask = 0x80000000u;
        /// Mask used to extract reference index from node.
        static constexpr UInt indexMask = 0x7FFFFFFFu;

        /// Return whether node is leaf.
        [[nodiscard]] bool IsLeaf() const noexcept {
            return (second & leafMask) != 0u;
        }

        /// Return left child index.
        [[nodiscard]] UInt LeftChild() const noexcept { return first; }
        /// Return right child index.
        [[nodiscard]] UInt RightChild() const noexcept {
            return second & indexMask;
        }
        /// Return first reference index.
        [[nodiscard]] UInt FirstReference() const noexcept { return first; }
        /// Return reference count.
        [[nodiscard]] UInt ReferenceCount() const noexcept {
            return second & indexMask;
        }

        /// Set node as leaf with given reference index and count.
        void SetLeaf(const UInt firstRef, const UInt count) noexcept {
            first = firstRef;
            second = leafMask | (count & indexMask);
        }

        /// Set node as inner node with given left and right child indices.
        void SetInner(const UInt left, const UInt right) noexcept {
            first = left;
            second = right & indexMask;
        }
    };

    static_assert(sizeof(Node) == 32, "Node must be 32 bytes");

    /// Reference used during BVH construction.
    struct Reference {
        /// Mesh or triangle index.
        UInt index{0};
        /// Bounding box.
        Bounds3f bounds;
    };

    /// Candidate split produced by SAH heuristic.
    struct Split {
        /// Type of split: object or spatial.
        enum class Type : UChar { None, Object, Spatial };
        /// Type of split.
        Type type{Type::None};
        /// Axis along which to split.
        UInt axis{0};
        /// Split position along axis.
        Float position{0.0f};
        /// Cost of split.
        Float cost{std::numeric_limits<Float>::infinity()};
        /// Overlap area between child bounds, used to gate spatial splits.
        Float overlap_area{0.0f};
    };

  public:
    /// Build two-level SBVH over given meshes.
    void Build(const std::vector<Mesh> &meshes) {
        meshes_ = &meshes;
        top_nodes_.clear();
        top_refs_.clear();
        bottom_nodes_.assign(meshes.size(), {});
        bottom_refs_.assign(meshes.size(), {});

        BuildTopLevel();
        for (UInt m = 0; m < meshes.size(); ++m)
            BuildBottomLevel(m);
    }

    /// Intersect `Ray` against BVH hierarchy and return closest triangle.
    [[nodiscard]] std::optional<TriangleIntersection3f>
    Intersect(const Ray3f &ray) const noexcept {
        return Intersect(ray, [](UInt) noexcept { return false; });
    }

    /// Intersect `Ray` against BVH hierarchy, skipping mesh for which
    /// `Predicate` is true.
    template <typename Predicate>
        requires std::predicate<Predicate, UInt>
    [[nodiscard]] std::optional<TriangleIntersection3f>
    Intersect(const Ray3f &ray, Predicate skip_mesh) const
        noexcept(std::is_nothrow_invocable_v<Predicate &, UInt>) {
        return IntersectMeshes(ray, skip_mesh);
    }

    /// Return bounding box of BVH hierarchy.
    [[nodiscard]] const Bounds3f &Bounds() const noexcept { return bounds_; }

  private:
    /// Number of bins for spatial split cost estimation.
    static constexpr UInt num_bins = 64;
    /// Maximum leaf size for bottom-level BVH.
    static constexpr UInt max_leaf_size = 8;
    /// Maximum depth of BVH.
    static constexpr UInt max_depth = 64;
    /// Alpha value for spatial split cost estimation.
    static constexpr Float spatial_split_alpha = 1e-5f;

    /// Meshes.
    const std::vector<Mesh> *meshes_{nullptr};

    /// Top-level BVH nodes over meshes.
    std::vector<Node> top_nodes_;
    /// Top-level leaf references.
    std::vector<UInt> top_refs_;
    /// Per-mesh bottom-level BVH nodes over triangles.
    std::vector<std::vector<Node>> bottom_nodes_;
    /// Per-mesh bottom-level leaf references.
    std::vector<std::vector<UInt>> bottom_refs_;
    /// Union of all mesh bounds.
    Bounds3f bounds_;

    /// Build top-level BVH over meshes.
    void BuildTopLevel() {
        const std::vector<Mesh> &meshes = *meshes_;
        if (meshes.empty()) {
            bounds_ = Bounds3f{};
            return;
        }

        std::vector<Reference> references =
            std::views::iota(0u, static_cast<UInt>(meshes.size())) |
            std::views::transform([&](const UInt m) {
                return Reference{m, MeshBounds(meshes[m])};
            }) |
            std::ranges::to<std::vector>();

        bounds_ = Union(references);
        BuildRecursively(references, top_nodes_, top_refs_, 0, 1, false,
                         nullptr);
    }

    /// Build bottom-level BVH for single mesh.
    void BuildBottomLevel(const UInt mesh_index) {
        const Mesh &mesh = (*meshes_)[mesh_index];

        std::vector<Reference> references =
            std::views::iota(0u, static_cast<UInt>(mesh.triangles.size())) |
            std::views::transform([&](const UInt t) {
                return Reference{t, TriangleBounds(mesh, t)};
            }) |
            std::ranges::to<std::vector>();

        auto &nodes = bottom_nodes_[mesh_index];
        auto &leaf_refs = bottom_refs_[mesh_index];
        nodes.clear();
        leaf_refs.clear();
        if (!references.empty())
            BuildRecursively(references, nodes, leaf_refs, 0, max_leaf_size,
                             true, &mesh);
    }

    /// Recursively subdivide references into BVH subtree rooted at freshly
    /// appended node.
    UInt BuildRecursively(std::vector<Reference> &references,
                          std::vector<Node> &nodes,
                          std::vector<UInt> &leaf_refs, const UInt depth,
                          const UInt leaf_size, const bool allow_spatial,
                          const Mesh *mesh) {
        const UInt node_index = static_cast<UInt>(nodes.size());
        nodes.emplace_back();
        const Bounds3f node_bounds = Union(references);
        nodes[node_index].bounds = node_bounds;

        if (references.size() <= leaf_size || depth >= max_depth ||
            !node_bounds.IsValid()) {
            MakeLeaf(node_index, references, nodes, leaf_refs);
            return node_index;
        }

        const Float leaf_cost = static_cast<Float>(references.size());
        const Split object_split = FindObjectSplit(references, node_bounds);

        // Only consider spatial splits once child nodes start overlapping.
        const Float root_area =
            bounds_.IsValid() ? std::max(bounds_.Area3D(), Epsilon) : Epsilon;
        const bool consider_spatial =
            allow_spatial && object_split.type != Split::Type::None &&
            object_split.overlap_area / root_area > spatial_split_alpha;
        const Split spatial_split =
            consider_spatial ? FindSpatialSplit(references, node_bounds)
                             : Split();

        const Split &best = spatial_split.cost < object_split.cost
                                ? spatial_split
                                : object_split;

        std::vector<Reference> left, right;
        if (best.type != Split::Type::None && best.cost < leaf_cost) {
            if (best.type == Split::Type::Spatial)
                ApplySpatialSplit(references, best, left, right, mesh);
            else
                ApplyObjectSplit(references, best, left, right);
        }

        // Median fallback when SAH split failed to partition.
        if ((left.empty() || right.empty()) && references.size() > 1) {
            left.clear();
            right.clear();
            ApplyMedianSplit(references, left, right);
        }

        // Make leaf if splitting still cannot make progress.
        if (left.empty() || right.empty() ||
            (left.size() == references.size() &&
             right.size() == references.size())) {
            MakeLeaf(node_index, references, nodes, leaf_refs);
            return node_index;
        }

        const UInt left_child = BuildRecursively(
            left, nodes, leaf_refs, depth + 1, leaf_size, allow_spatial, mesh);
        const UInt right_child = BuildRecursively(
            right, nodes, leaf_refs, depth + 1, leaf_size, allow_spatial, mesh);
        nodes[node_index].SetInner(left_child, right_child);
        return node_index;
    }

    /// Turn node into leaf pointing at references.
    void MakeLeaf(const UInt node_index,
                  const std::vector<Reference> &references,
                  std::vector<Node> &nodes,
                  std::vector<UInt> &leaf_refs) const {
        const UInt first_reference = static_cast<UInt>(leaf_refs.size());
        for (const auto &reference : references)
            leaf_refs.push_back(reference.index);
        nodes[node_index].SetLeaf(first_reference,
                                  static_cast<UInt>(references.size()));
    }

    /// Find best binned object split per SAH.
    Split FindObjectSplit(const std::vector<Reference> &references,
                          const Bounds3f &node_bounds) const {
        Split best;
        if (references.size() < 2 || !node_bounds.IsValid())
            return best;

        const Float node_area = std::max(node_bounds.Area3D(), Epsilon);
        const Bounds3f centroid_bounds = CentroidBounds(references);

        struct Bin {
            Bounds3f bounds;
            UInt count{0};
        };

        std::array<Bin, num_bins> bins;
        std::array<Bounds3f, num_bins - 1> left_bounds;
        std::array<Bounds3f, num_bins - 1> right_bounds;
        std::array<UInt, num_bins - 1> left_counts;
        std::array<UInt, num_bins - 1> right_counts;

        for (UInt axis = 0; axis < 3; ++axis) {
            const Float extent = centroid_bounds.Size(static_cast<Int>(axis));
            if (extent <= Epsilon)
                continue;

            std::ranges::fill(bins, Bin{});

            const Float scale = static_cast<Float>(num_bins) / extent;
            for (const auto &reference : references) {
                const Float centroid = Centroid(reference, axis);
                UInt bin_index = static_cast<UInt>(
                    (centroid - centroid_bounds.min[axis]) * scale);
                if (bin_index >= num_bins)
                    bin_index = num_bins - 1;
                bins[bin_index].count++;
                bins[bin_index].bounds.Expand(reference.bounds);
            }

            Bounds3f running;
            UInt running_count = 0;
            for (UInt i = 0; i < num_bins - 1; ++i) {
                running_count += bins[i].count;
                running.Expand(bins[i].bounds);
                left_counts[i] = running_count;
                left_bounds[i] = running;
            }

            running = Bounds3f{};
            running_count = 0;
            for (Int i = static_cast<Int>(num_bins) - 1; i > 0; --i) {
                running_count += bins[i].count;
                running.Expand(bins[i].bounds);
                right_counts[i - 1] = running_count;
                right_bounds[i - 1] = running;
            }

            const Float step = extent / static_cast<Float>(num_bins);
            for (UInt i = 0; i < num_bins - 1; ++i) {
                if (left_counts[i] == 0 || right_counts[i] == 0)
                    continue;
                const Float cost = (SurfaceArea(left_bounds[i]) *
                                        static_cast<Float>(left_counts[i]) +
                                    SurfaceArea(right_bounds[i]) *
                                        static_cast<Float>(right_counts[i])) /
                                   node_area;

                if (cost < best.cost) {
                    Bounds3f overlap = left_bounds[i];
                    overlap.Intersect(right_bounds[i]);
                    best.type = Split::Type::Object;
                    best.axis = axis;
                    best.position = centroid_bounds.min[axis] + (i + 1) * step;
                    best.cost = cost;
                    best.overlap_area =
                        overlap.IsValid() ? std::max(Float{0}, overlap.Area3D())
                                          : Float{0};
                }
            }
        }

        return best;
    }

    /// Find best binned spatial split per SAH.  References straddling split
    /// plane contribute to both sides via clipped bounds.
    Split FindSpatialSplit(const std::vector<Reference> &references,
                           const Bounds3f &node_bounds) const {
        Split best;
        if (references.size() < 2 || !node_bounds.IsValid())
            return best;

        const Float node_area = std::max(node_bounds.Area3D(), Epsilon);

        for (UInt axis = 0; axis < 3; ++axis) {
            const Float extent = node_bounds.Size(static_cast<Int>(axis));
            if (extent <= Epsilon)
                continue;

            const Float step = extent / static_cast<Float>(num_bins);
            for (UInt i = 0; i < num_bins - 1; ++i) {
                const Float split_position =
                    node_bounds.min[axis] + (i + 1) * step;

                Bounds3f left_bounds, right_bounds;
                UInt left_count = 0;
                UInt right_count = 0;
                for (const auto &reference : references) {
                    if (reference.bounds.max[axis] <= split_position) {
                        left_bounds.Expand(reference.bounds);
                        ++left_count;
                    } else if (reference.bounds.min[axis] >= split_position) {
                        right_bounds.Expand(reference.bounds);
                        ++right_count;
                    } else {
                        Bounds3f left_clip = reference.bounds;
                        Bounds3f right_clip = reference.bounds;
                        left_clip.max[axis] = split_position;
                        right_clip.min[axis] = split_position;
                        left_bounds.Expand(left_clip);
                        right_bounds.Expand(right_clip);
                        ++left_count;
                        ++right_count;
                    }
                }

                if (left_count == 0 || right_count == 0)
                    continue;

                const Float cost =
                    (SurfaceArea(left_bounds) * static_cast<Float>(left_count) +
                     SurfaceArea(right_bounds) *
                         static_cast<Float>(right_count)) /
                    node_area;

                if (cost < best.cost) {
                    best.type = Split::Type::Spatial;
                    best.axis = axis;
                    best.position = split_position;
                    best.cost = cost;
                }
            }
        }

        return best;
    }

    /// Partition references by centroid relative to split plane.
    void ApplyObjectSplit(const std::vector<Reference> &references,
                          const Split &split, std::vector<Reference> &left,
                          std::vector<Reference> &right) const {
        std::ranges::partition_copy(
            references, std::back_inserter(left), std::back_inserter(right),
            [&](const Reference &r) {
                return Centroid(r, split.axis) < split.position;
            });
    }

    /// Split straddling references, clipping duplicated triangles against
    /// splitting plane to tighten child bounds.
    void ApplySpatialSplit(const std::vector<Reference> &references,
                           const Split &split, std::vector<Reference> &left,
                           std::vector<Reference> &right,
                           const Mesh *mesh) const {
        left.reserve(references.size());
        right.reserve(references.size());

        Bounds3f left_bounds, right_bounds;

        // References lying entirely on one side of splitting plane.
        for (const auto &reference : references) {
            if (reference.bounds.max[split.axis] <= split.position) {
                left.push_back(reference);
                left_bounds.Expand(reference.bounds);
            } else if (reference.bounds.min[split.axis] >= split.position) {
                right.push_back(reference);
                right_bounds.Expand(reference.bounds);
            }
        }

        auto Expand = [](Bounds3f bounds, const Bounds3f &other) {
            bounds.Expand(other);
            return bounds;
        };

        // Straddling references are unsplit to one side, or clipped and
        // duplicated based on local SAH cost of each option.
        for (const auto &reference : references) {
            if (reference.bounds.max[split.axis] <= split.position ||
                reference.bounds.min[split.axis] >= split.position)
                continue;

            Bounds3f left_bounds = reference.bounds;
            Bounds3f right_bounds = reference.bounds;
            left_bounds.max[split.axis] = split.position;
            right_bounds.min[split.axis] = split.position;

            Reference left_reference = reference;
            Reference right_reference = reference;
            if (mesh != nullptr) {
                left_reference.bounds =
                    ClipTriangle(reference.index, *mesh, left_bounds);
                right_reference.bounds =
                    ClipTriangle(reference.index, *mesh, right_bounds);
            } else {
                left_reference.bounds = left_bounds;
                right_reference.bounds = right_bounds;
            }

            const Float left_count = static_cast<Float>(left.size());
            const Float right_count = static_cast<Float>(right.size());

            const Bounds3f unsplit_left_bounds =
                Expand(left_bounds, reference.bounds);
            const Bounds3f unsplit_right_bounds =
                Expand(right_bounds, reference.bounds);
            const Bounds3f duplicate_left_bounds =
                Expand(left_bounds, left_reference.bounds);
            const Bounds3f duplicate_right_bounds =
                Expand(right_bounds, right_reference.bounds);

            const Float unsplit_left_cost =
                SurfaceArea(unsplit_left_bounds) * (left_count + 1.0f) +
                SurfaceArea(right_bounds) * right_count;
            const Float unsplit_right_cost =
                SurfaceArea(left_bounds) * left_count +
                SurfaceArea(unsplit_right_bounds) * (right_count + 1.0f);
            const Float duplicate_cost =
                SurfaceArea(duplicate_left_bounds) * (left_count + 1.0f) +
                SurfaceArea(duplicate_right_bounds) * (right_count + 1.0f);

            if (unsplit_left_cost <= unsplit_right_cost &&
                unsplit_left_cost <= duplicate_cost) {
                left.push_back(reference);
                left_bounds = unsplit_left_bounds;
            } else if (unsplit_right_cost <= duplicate_cost) {
                right.push_back(reference);
                right_bounds = unsplit_right_bounds;
            } else {
                left.push_back(left_reference);
                right.push_back(right_reference);
                left_bounds = duplicate_left_bounds;
                right_bounds = duplicate_right_bounds;
            }
        }
    }

    /// Median split fallback along widest centroid axis.
    void ApplyMedianSplit(std::vector<Reference> &references,
                          std::vector<Reference> &left,
                          std::vector<Reference> &right) const {
        const Bounds3f centroid_bounds = CentroidBounds(references);
        const UInt axis =
            centroid_bounds.IsValid() ? centroid_bounds.MajorAxis() : 0u;

        std::ranges::sort(references, {}, [&](const Reference &r) {
            return Centroid(r, axis);
        });

        const std::size_t split_index = references.size() / 2;
        left.assign(references.begin(), references.begin() + split_index);
        right.assign(references.begin() + split_index, references.end());
    }

    /// Union of all reference bounds.
    Bounds3f Union(const std::vector<Reference> &references) const {
        return std::ranges::fold_left(
            references | std::views::transform(&Reference::bounds), Bounds3f{},
            [](Bounds3f acc, const Bounds3f &b) {
                acc.Expand(b);
                return acc;
            });
    }

    /// Surface area clamped to zero for degenerate bounds.
    [[nodiscard]] static Float SurfaceArea(const Bounds3f &bounds) noexcept {
        return bounds.IsValid() ? std::max(Float{0}, bounds.Area3D())
                                : Float{0};
    }

    /// Centroid coordinate of reference along given axis.
    [[nodiscard]] static Float Centroid(const Reference &reference,
                                        const UInt axis) noexcept {
        return 0.5f * (reference.bounds.min[axis] + reference.bounds.max[axis]);
    }

    /// Bounds of all reference centroids.
    Bounds3f CentroidBounds(const std::vector<Reference> &references) const {
        return std::ranges::fold_left(
            references | std::views::transform([](const Reference &r) {
                return r.bounds.Center();
            }),
            Bounds3f{}, [](Bounds3f acc, const Point3f &c) {
                acc.Expand(c);
                return acc;
            });
    }

    /// Per-triangle bounds.
    Bounds3f TriangleBounds(const Mesh &mesh, const UInt triangle_index) const {
        const Triangle &triangle = mesh.triangles[triangle_index];
        Bounds3f bounds;
        bounds.Expand(mesh.vertices[triangle.a]);
        bounds.Expand(mesh.vertices[triangle.b]);
        bounds.Expand(mesh.vertices[triangle.c]);
        return bounds;
    }

    /// Per-mesh bounds.
    Bounds3f MeshBounds(const Mesh &mesh) const {
        return std::ranges::fold_left(
            std::views::iota(0u, static_cast<UInt>(mesh.triangles.size())) |
                std::views::transform(
                    [&](const UInt t) { return TriangleBounds(mesh, t); }),
            Bounds3f{}, [](Bounds3f acc, const Bounds3f &b) {
                acc.Expand(b);
                return acc;
            });
    }

    /// Clip triangle to axis-aligned bounds using Sutherland-Hodgman.
    Bounds3f ClipTriangle(const UInt triangle_index, const Mesh &mesh,
                          const Bounds3f &box) const {
        if (!box.IsValid())
            return {};

        const Triangle &triangle = mesh.triangles[triangle_index];

        static constexpr UInt max_vertices = 32;
        std::array<Point3f, max_vertices> polygon;
        std::array<Point3f, max_vertices> output;
        UInt vertex_count = 3;
        polygon[0] = mesh.vertices[triangle.a];
        polygon[1] = mesh.vertices[triangle.b];
        polygon[2] = mesh.vertices[triangle.c];

        auto ClipPlane = [&](const UInt axis, const Float plane,
                             const bool keep_greater) {
            if (vertex_count == 0)
                return;
            UInt output_count = 0;
            Point3f previous = polygon[vertex_count - 1];
            bool previous_inside = keep_greater
                                       ? previous[axis] >= plane - Epsilon
                                       : previous[axis] <= plane + Epsilon;
            for (UInt i = 0; i < vertex_count && output_count < max_vertices;
                 ++i) {
                const Point3f current = polygon[i];
                const bool current_inside =
                    keep_greater ? current[axis] >= plane - Epsilon
                                 : current[axis] <= plane + Epsilon;

                if (current_inside != previous_inside) {
                    const Float denominator = current[axis] - previous[axis];
                    if (std::fabs(denominator) > 1e-8f) {
                        Float t = (plane - previous[axis]) / denominator;
                        t = std::max(Float{0}, std::min(Float{1}, t));
                        Point3f intersection;
                        for (UInt k = 0; k < 3; ++k)
                            intersection[k] =
                                previous[k] + (current[k] - previous[k]) * t;
                        output[output_count++] = intersection;
                    }
                }

                if (current_inside)
                    output[output_count++] = current;

                previous = current;
                previous_inside = current_inside;
            }

            for (UInt i = 0; i < output_count; ++i)
                polygon[i] = output[i];
            vertex_count = output_count;
        };

        for (UInt axis = 0; axis < 3; ++axis) {
            ClipPlane(axis, box.min[axis], true);
            ClipPlane(axis, box.max[axis], false);
        }

        if (vertex_count == 0)
            return {};

        Bounds3f result;
        for (UInt i = 0; i < vertex_count; ++i)
            result.Expand(polygon[i]);
        result.Intersect(box);
        return result.IsValid() ? result : Bounds3f{};
    }

    /// Intersect `Ray` against top-level BVH, skipping mesh for which
    /// `Predicate` is true.
    template <typename Predicate>
        requires std::predicate<Predicate, UInt>
    std::optional<TriangleIntersection3f>
    IntersectMeshes(const Ray3f &ray, Predicate &skip_mesh) const
        noexcept(std::is_nothrow_invocable_v<Predicate &, UInt>) {
        std::optional<TriangleIntersection3f> closest;
        if (top_nodes_.empty() || meshes_ == nullptr)
            return closest;

        Float near_t, far_t;
        if (!bounds_.Intersect(ray, near_t, far_t))
            return closest;
        near_t = std::max(near_t, Float{0});
        if (near_t > far_t)
            return closest;

        Float max_t = std::numeric_limits<Float>::infinity();
        Traverse(top_nodes_, ray, near_t, max_t,
                 [&](const UInt first, const UInt count) {
                     for (UInt i = 0; i < count; ++i) {
                         const UInt mesh_index = top_refs_[first + i];
                         if (skip_mesh(mesh_index))
                             continue;
                         IntersectTriangles(ray, mesh_index, closest, max_t);
                     }
                 });

        return closest;
    }

    /// Intersect `Ray` against bottom-level BVH.
    void IntersectTriangles(const Ray3f &ray, const UInt mesh_index,
                            std::optional<TriangleIntersection3f> &closest,
                            Float &max_t) const noexcept {
        const std::vector<Node> &nodes = bottom_nodes_[mesh_index];
        if (nodes.empty())
            return;
        const Mesh &mesh = (*meshes_)[mesh_index];
        const std::vector<UInt> &leaf_refs = bottom_refs_[mesh_index];

        Float near_t, far_t;
        if (!nodes[0].bounds.Intersect(ray, near_t, far_t))
            return;
        near_t = std::max(near_t, Float{0});
        if (near_t > far_t || near_t > max_t)
            return;

        Traverse(
            nodes, ray, near_t, max_t, [&](const UInt first, const UInt count) {
                for (UInt i = 0; i < count; ++i) {
                    const UInt triangle_index = leaf_refs[first + i];
                    const Triangle &triangle = mesh.triangles[triangle_index];
                    const auto intersection = triangle.Intersect(
                        ray, mesh.vertices[triangle.a],
                        mesh.vertices[triangle.b], mesh.vertices[triangle.c]);
                    if (intersection &&
                        (!closest || intersection->t < closest->t) &&
                        intersection->t <= max_t) {
                        closest = intersection;
                        closest->mesh_index = mesh_index;
                        closest->triangle_index = triangle_index;
                        max_t = intersection->t;
                    }
                }
            });
    }

    /// Generic depth-first BVH traversal shared by top-level and bottom-level
    /// intersections.
    template <typename LeafHandler>
        requires std::invocable<LeafHandler &, UInt, UInt>
    static void Traverse(const std::vector<Node> &nodes, const Ray3f &ray,
                         Float start_near, Float &max_t,
                         LeafHandler process) noexcept {
        struct StackEntry {
            UInt node;
            Float near_t;
        };

        std::array<StackEntry, max_depth + 1> stack;
        UInt stack_size = 0;
        stack[stack_size++] = {0u, start_near};

        while (stack_size > 0) {
            const StackEntry entry = stack[--stack_size];
            if (entry.near_t > max_t)
                continue;

            const Node &node = nodes[entry.node];
            if (node.IsLeaf()) {
                process(node.FirstReference(), node.ReferenceCount());
                continue;
            }

            const UInt li = node.LeftChild();
            const UInt ri = node.RightChild();
            const Node &lc = nodes[li];
            const Node &rc = nodes[ri];
            Float ln, lf, rn, rf;
            bool hl = lc.bounds.Intersect(ray, ln, lf);
            bool hr = rc.bounds.Intersect(ray, rn, rf);
            if (hl) {
                ln = std::max(ln, Float{0});
                hl = ln <= lf;
            }
            if (hr) {
                rn = std::max(rn, Float{0});
                hr = rn <= rf;
            }

            if (hl && hr) {
                // Visit nearer child first so further child can be pruned.
                if (ln < rn) {
                    stack[stack_size++] = {ri, rn};
                    stack[stack_size++] = {li, ln};
                } else {
                    stack[stack_size++] = {li, ln};
                    stack[stack_size++] = {ri, rn};
                }
            } else if (hl) {
                stack[stack_size++] = {li, ln};
            } else if (hr) {
                stack[stack_size++] = {ri, rn};
            }
        }
    }
};

} // namespace rays
