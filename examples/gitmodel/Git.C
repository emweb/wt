/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Git.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <ctype.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>

/*
 * Small utility methods and classes.
 */
namespace {
  unsigned char fromHex(char b)
  {
    if (b <= '9')
      return b - '0';
    else if (b <= 'F')
      return (b - 'A') + 0x0A;
    else 
      return (b - 'a') + 0x0A;
  }

  unsigned char fromHex(char msb, char lsb)
  {
    return (fromHex(msb) << 4) + fromHex(lsb);
  }

  char toHex(unsigned char b)
  {
    if (b < 0xA)
      return '0' + b;
    else
      return 'a' + (b - 0xA);
  }

  void toHex(unsigned char b, char& msb, char& lsb)
  {
    lsb = toHex(b & 0x0F);
    msb = toHex(b >> 4);
  }

  /*
   * Run a command and capture its stdout into a string.
   * Uses and maintains a cache.
   */
  class POpenWrapper
  {
  public:
    POpenWrapper(const std::string& s, Git::Cache& cache) {
      bool cached = false;

      for (Git::Cache::iterator i = cache.begin(); i != cache.end(); ++i)
	if (i->first == s) {
	  content_ = i->second;
	  status_ = 0;
	  cached = true;
	  cache.splice(cache.begin(), cache, i); // implement LRU
	  break;
	}

      if (!cached) {
	std::cerr << s << std::endl;
	FILE *stream = popen((s + "  2>&1").c_str(), "r");
	if (!stream)
	  throw Git::Exception("Git: could not execute: '" + s + "'");

	int n = 0;
	do {
	  char buffer[32000];
	  n = fread(buffer, 1, 30000, stream);
	  buffer[n] = 0;
	  content_ += std::string(buffer, n);
	} while (n);

	status_ = pclose(stream);

	if (status_ == 0) {
	  cache.pop_back(); // implement LRU
	  cache.push_front(std::make_pair(s, content_));
	}
      }

      idx_ = 0;
    }

    std::string& readLine(std::string& r, bool stripWhite = true) {
      r.clear();

      while (stripWhite
	     && (idx_ < content_.length()) && isspace(content_[idx_]))
	++idx_;

      while (idx_ < content_.size() && content_[idx_] != '\n') {
	r += content_[idx_];
	++idx_;
      }

      if (idx_ < content_.size())
	++idx_;

      return r;
    }

    const std::string& contents() const {
      return content_;
    }

    bool finished() const {
      return idx_ == content_.size();
    }

    int exitStatus() const {
      return status_;
    }

  private:
    std::string content_;
    unsigned int idx_;
    int status_;
  };
}

/*
 * About the git files:
 * type="commit":
 *  - of a reference, like the SHA1 ID obtained from git-rev-parse of a
 *    particular revision
 *  - contains the SHA1 ID of the tree
 *
 * type="tree":
 *  100644 blob 0732f5e4def48d6d5b556fbad005adc994af1e0b    CMakeLists.txt
 *  040000 tree 037d59672d37e116f6e0013a067a7ce1f8760b7c    Wt
 *  <mode> SP <type> SP <object> TAB <file>
 *
 * type="blob": contents of a file
 */

Git::Exception::Exception(const std::string& msg)
  : std::runtime_error(msg)
{ }

Git::ObjectId::ObjectId()
{ }

Git::ObjectId::ObjectId(const std::string& id)
{
  if (id.length() != 40)
    throw Git::Exception("Git: not a valid SHA1 id: " + id);

  for (int i = 0; i < 20; ++i)
    (*this)[i] = fromHex(id[2 * i], id[2 * i + 1]);
}

std::string Git::ObjectId::toString() const
{
  std::string result(40, '-');

  for (int i = 0; i < 20; ++i)
    toHex((*this)[i], result[2 * i], result[2 * i + 1]);

  return result;
}

Git::Object::Object(const ObjectId& anId, ObjectType aType)
  : id(anId),
    type(aType)
{ }

Git::Git()
  : cache_(3) // cache of 3 git results
{ }

void Git::setRepositoryPath(const std::string& repositoryPath)
{ 
  repository_ = repositoryPath;
  checkRepository();
}

Git::ObjectId Git::getCommitTree(const std::string& revision) const
{
  Git::ObjectId commit = getCommit(revision);
  return getTreeFromCommit(commit);
}

std::string Git::catFile(const ObjectId& id) const
{
  std::string result;

  if (!getCmdResult("cat-file -p " + id.toString(), result, -1))
    throw Exception("Git: could not cat '" + id.toString() + "'");

  return result;
}

Git::ObjectId Git::getCommit(const std::string& revision) const
{
  std::string sha1Commit;
  getCmdResult("rev-parse " + revision, sha1Commit, 0);
  return ObjectId(sha1Commit);
}

Git::ObjectId Git::getTreeFromCommit(const ObjectId& commit) const
{
  std::string treeLine;
  if (!getCmdResult("cat-file -p " + commit.toString(), treeLine, "tree"))
    throw Exception("Git: could not parse tree from commit '" 
		    + commit.toString() + "'");

  std::vector<std::string> v;
  boost::split(v, treeLine, boost::is_any_of(" "));
  if (v.size() != 2)
    throw Exception("Git: could not parse tree from commit '"
		    + commit.toString() + "': '" + treeLine + "'");
  return ObjectId(v[1]);
}

Git::Object Git::treeGetObject(const ObjectId& tree, int index) const
{
  std::string objectLine;
  if (!getCmdResult("cat-file -p " + tree.toString(), objectLine, index))
    throw Exception("Git: could not read object %"
		    + boost::lexical_cast<std::string>(index)
		    + "  from tree " + tree.toString());
  else {
    std::vector<std::string> v1, v2;
    boost::split(v1, objectLine, boost::is_any_of("\t"));
    if (v1.size() != 2)
      throw Exception("Git: could not parse tree object line: '"
		      + objectLine + "'");
    boost::split(v2, v1[0], boost::is_any_of(" "));
    if (v2.size() != 3)
      throw Exception("Git: could not parse tree object line: '"
		      + objectLine + "'");
 
    const std::string& stype = v2[1];
    ObjectType type;
    if (stype == "tree")
      type = Tree;
    else if (stype == "blob")
      type = Blob;
    else
      throw Exception("Git: Unknown type: " + stype);

    Git::Object result(ObjectId(v2[2]), type);
    result.name = v1[1];

    return result;
  }
}

int Git::treeSize(const ObjectId& tree) const
{
  return getCmdResultLineCount("cat-file -p " + tree.toString());
}

bool Git::getCmdResult(const std::string& gitCmd, std::string& result,
		       int index) const
{
  POpenWrapper p("git --git-dir=" + repository_ + " " + gitCmd, cache_);

  if (p.exitStatus() != 0)
    throw Exception("Git error: " + p.readLine(result));

  if (index == -1) {
    result = p.contents();
    return true;
  } else
    p.readLine(result);

  for (int i = 0; i < index; ++i) {
    if (p.finished())
      return false;
    p.readLine(result);
  }

  return true;
}

bool Git::getCmdResult(const std::string& gitCmd, std::string& result,
		       const std::string& tag) const
{
  POpenWrapper p("git --git-dir=" + repository_ + " " + gitCmd, cache_);

  if (p.exitStatus() != 0)
    throw Exception("Git error: " + p.readLine(result));

  while (!p.finished()) {
    p.readLine(result);
    if (boost::starts_with(result, tag))
      return true;
  }

  return false;
}

int Git::getCmdResultLineCount(const std::string& gitCmd) const
{
  POpenWrapper p("git --git-dir=" + repository_ + " " + gitCmd, cache_);

  std::string r;

  if (p.exitStatus() != 0)
    throw Exception("Git error: " + p.readLine(r));

  int result = 0;
  while (!p.finished()) {
    p.readLine(r);
    ++result;
  }

  return result;
}

void Git::checkRepository() const
{
  POpenWrapper p("git --git-dir=" + repository_ + " branch", cache_);

  std::string r;
  if (p.exitStatus() != 0)
    throw Exception("Git error: " + p.readLine(r));
}
