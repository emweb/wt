#include <Wt/WAbstractItemModel.h>

#include "Git.h"

class GitModel : public Wt::WAbstractItemModel
{
public:
    /*
     * A custom role for the file contents of a Git BLOB object.
     */
#ifndef WT_TARGET_JAVA
    static constexpr Wt::ItemDataRole ContentsRole = Wt::ItemDataRole::User + 1;
#else
    static constexpr Wt::ItemDataRole ContentsRole = Wt::ItemDataRole(Wt::ItemDataRole::User.value() + 1);
#endif

    GitModel(const std::string& repository)
        : WAbstractItemModel()
    { 
	git_.setRepositoryPath(repository);
	loadRevision("master");
    }

    void loadRevision(const std::string& revName) {
	Git::ObjectId treeRoot = git_.getCommitTree(revName);

	layoutAboutToBeChanged().emit(); // Invalidates model indexes

	treeData_.clear();
	childPointer_.clear();

	/*
	 * This stores the tree root as treeData_[0]
	 */
	treeData_.push_back(Tree(-1, -1, treeRoot, git_.treeSize(treeRoot)));

	layoutChanged().emit();
    }

    virtual Wt::WModelIndex parent(const Wt::WModelIndex& index) const {
	if (!index.isValid() || index.internalId() == 0) {
	    return Wt::WModelIndex(); // treeData_[0] is the tree root
	} else {
	    const Tree& item = treeData_[index.internalId()];
	    return createIndex(item.index(), 0, item.parentId());
	}
    }

    virtual Wt::WModelIndex index(int row, int column,
                                  const Wt::WModelIndex& parent = Wt::WModelIndex()) const {
	int parentId;

	if (!parent.isValid())
	    parentId = 0;
	else {
	    int grandParentId = parent.internalId();
	    parentId = getTreeId(grandParentId, parent.row());
	}

	return createIndex(row, column, parentId);
    }

    virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const {
	return 2;
    }

    virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const {
	int treeId;

	if (parent.isValid()) {
	    if (parent.column() != 0)
		return 0;
	    Git::Object o = getObject(parent);
	    if (o.type == Git::Tree) { // is a folder
		treeId = getTreeId(parent.internalId(), parent.row());
	    } else                     // is a file
		return 0;
	} else {
	    treeId = 0;
	}

	return treeData_[treeId].rowCount();
    }

    virtual Wt::cpp17::any data(const Wt::WModelIndex& index, Wt::ItemDataRole role = Wt::ItemDataRole::Display) const {
	if (!index.isValid())
	    return Wt::cpp17::any();

	Git::Object object = getObject(index);

	switch (index.column()) {
	case 0:
	    if (role == Wt::ItemDataRole::Display) {
		if (object.type == Git::Tree)
		    return object.name + '/';
		else
		    return object.name;
	    } else if (role == Wt::ItemDataRole::Decoration) {
		if (object.type == Git::Blob)
		    return std::string("icons/git-blob.png");
		else if (object.type == Git::Tree)
		    return std::string("icons/git-tree.png");
	    } else if (role == ContentsRole) {
		if (object.type == Git::Blob)
		    return git_.catFile(object.id);
	    }

	    break;
	case 1:
	    if (role == Wt::ItemDataRole::Display) {
		if (object.type == Git::Tree)
		    return std::string("Folder");
		else {
		    std::string suffix = getSuffix(object.name);

		    if (suffix == "C" || suffix == "cpp")
			return std::string("C++ Source");
		    else if (suffix == "h" || 
			     (suffix == "" && !topLevel(index)))
			return std::string("C++ Header");
		    else if (suffix == "css")
			return std::string("CSS Stylesheet");
		    else if (suffix == "js")
			return std::string("JavaScript Source");
		    else if (suffix == "md")
			return std::string("Markdown");
		    else if (suffix == "png" || suffix == "gif")
			return std::string("Image");
		    else if (suffix == "txt")
			return std::string("Text");
		    else
			return Wt::cpp17::any();
		}
	    }
	}

        return Wt::cpp17::any();
    }
  
    virtual Wt::cpp17::any headerData(int section,
                                  Wt::Orientation orientation = Wt::Orientation::Horizontal,
                                  Wt::ItemDataRole role = Wt::ItemDataRole::Display) const {
        if (orientation == Wt::Orientation::Horizontal && role == Wt::ItemDataRole::Display) {
	    switch (section) {
	    case 0:
		return std::string("File");
	    case 1:
		return std::string("Type");
	    default:
		return Wt::cpp17::any();
	    }
	} else
	    return Wt::cpp17::any();
    }

private:
    Git git_;

    /*
     * Identifies a folder given parent and index
     */
    struct ChildIndex {
	int parentId;
	int index;

	ChildIndex(int aParent, int anIndex)
	    : parentId(aParent), index(anIndex) { }

	bool operator< (const ChildIndex& other) const {
	    if (parentId < other.parentId)
		return true;
	    else if (parentId > other.parentId)
		return false;
	    else return index < other.index;
	}

#ifdef WT_TARGET_JAVA
	bool equals(Wt::cpp17::any o) {
	    ChildIndex *other = Wt::cpp17::any_cast<ChildIndex *>(o);
	    return parentId == other->parentId &&
	        index == other->index;
	}

        int hashCode() {
	    int hash = 1;
	    hash = hash * 31 + parentId;
	    hash = hash * 31 + index;
	    return hash;
	}
#endif // WT_TARGET_JAVA
    };

    /*
     * Data to be stored for an (expanded) folder
     */
    class Tree {
    public:
	Tree(int parentId, int index, const Git::ObjectId& object, int rowCount)
	    : index_(parentId, index),
	      treeObject_(object),
	      rowCount_(rowCount)
	{ }

	int parentId() const { return index_.parentId; }
	int index() const { return index_.index; }
	const Git::ObjectId& treeObject() const { return treeObject_; }
	int rowCount() const { return rowCount_; }

    private:
	ChildIndex    index_;
	Git::ObjectId treeObject_;
	int           rowCount_;
    };

    typedef std::map<ChildIndex, int> ChildPointerMap;

    /*
     * Expanded folder data
     */
    mutable std::vector<Tree> treeData_;

    /*
     * Indexes into treeData_
     */
    mutable ChildPointerMap childPointer_;

    /*
     * Gets or allocates an id for a folder.
     */
    int getTreeId(int parentId, int childIndex) const {
	ChildIndex index(parentId, childIndex);

	ChildPointerMap::const_iterator i = childPointer_.find(index);
	if (i == childPointer_.end()) {
	    const Tree& parentItem = treeData_[parentId];
	    Git::Object o = git_.treeGetObject(parentItem.treeObject(), childIndex);
 
	    treeData_.push_back(Tree(parentId, childIndex, o.id,
				     git_.treeSize(o.id)));
	    int result = treeData_.size() - 1;
	    childPointer_[index] = result;
	    return result;
	} else
	    return i->second;
    }

    /*
     * Gets the Git::Object that corresponds to an index.
     */
    Git::Object getObject(const Wt::WModelIndex& index) const {
	int parentId = index.internalId();
	const Tree& parentItem = treeData_[parentId];
	return git_.treeGetObject(parentItem.treeObject(), index.row());
    }

    static std::string getSuffix(const std::string& fileName) {
	std::size_t dot = fileName.rfind('.');
	if (dot == std::string::npos)
	    return "";
	else
	    return fileName.substr(dot + 1);
    }

    bool topLevel(const Wt::WModelIndex& index) const {
	return !parent(index).isValid();
    }
};

#ifndef WT_TARGET_JAVA
constexpr Wt::ItemDataRole GitModel::ContentsRole;
#endif // WT_TARGET_JAVA

SAMPLE_BEGIN(gitModel)
SAMPLE_END(return nullptr)
