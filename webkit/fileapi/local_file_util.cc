// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webkit/fileapi/local_file_util.h"

#include "base/file_util.h"
#include "base/files/file_util_proxy.h"
#include "googleurl/src/gurl.h"
#include "webkit/browser/fileapi/file_system_mount_point_provider.h"
#include "webkit/fileapi/file_system_context.h"
#include "webkit/fileapi/file_system_operation_context.h"
#include "webkit/fileapi/file_system_types.h"
#include "webkit/fileapi/file_system_url.h"
#include "webkit/fileapi/file_system_util.h"
#include "webkit/fileapi/native_file_util.h"

namespace fileapi {

using base::PlatformFileError;

class LocalFileEnumerator : public FileSystemFileUtil::AbstractFileEnumerator {
 public:
  LocalFileEnumerator(const base::FilePath& platform_root_path,
                      const base::FilePath& virtual_root_path,
                      int file_type)
      : file_enum_(platform_root_path, false /* recursive */, file_type),
        platform_root_path_(platform_root_path),
        virtual_root_path_(virtual_root_path) {
#if defined(OS_WIN)
    memset(&file_util_info_, 0, sizeof(file_util_info_));
#endif  // defined(OS_WIN)
  }

  virtual ~LocalFileEnumerator() {}

  virtual base::FilePath Next() OVERRIDE;
  virtual int64 Size() OVERRIDE;
  virtual base::Time LastModifiedTime() OVERRIDE;
  virtual bool IsDirectory() OVERRIDE;

 private:
  file_util::FileEnumerator file_enum_;
  file_util::FileEnumerator::FindInfo file_util_info_;
  base::FilePath platform_root_path_;
  base::FilePath virtual_root_path_;
};

base::FilePath LocalFileEnumerator::Next() {
  base::FilePath next = file_enum_.Next();
  // Don't return symlinks.
  while (!next.empty() && file_util::IsLink(next))
    next = file_enum_.Next();
  if (next.empty())
    return next;
  file_enum_.GetFindInfo(&file_util_info_);

  base::FilePath path;
  platform_root_path_.AppendRelativePath(next, &path);
  return virtual_root_path_.Append(path);
}

int64 LocalFileEnumerator::Size() {
  return file_util::FileEnumerator::GetFilesize(file_util_info_);
}

base::Time LocalFileEnumerator::LastModifiedTime() {
  return file_util::FileEnumerator::GetLastModifiedTime(file_util_info_);
}

bool LocalFileEnumerator::IsDirectory() {
  return file_util::FileEnumerator::IsDirectory(file_util_info_);
}

LocalFileUtil::LocalFileUtil() {
}

LocalFileUtil::~LocalFileUtil() {
}

PlatformFileError LocalFileUtil::CreateOrOpen(
    FileSystemOperationContext* context,
    const FileSystemURL& url, int file_flags,
    base::PlatformFile* file_handle, bool* created) {
  *created = false;
  base::FilePath file_path;
  PlatformFileError error = GetLocalFilePath(context, url, &file_path);
  if (error != base::PLATFORM_FILE_OK)
    return error;
  // Disallow opening files in symlinked paths.
  if (file_util::IsLink(file_path))
    return base::PLATFORM_FILE_ERROR_NOT_FOUND;
  return NativeFileUtil::CreateOrOpen(
      file_path, file_flags, file_handle, created);
}

PlatformFileError LocalFileUtil::Close(FileSystemOperationContext* context,
                                       base::PlatformFile file) {
  return NativeFileUtil::Close(file);
}

PlatformFileError LocalFileUtil::EnsureFileExists(
    FileSystemOperationContext* context,
    const FileSystemURL& url,
    bool* created) {
  base::FilePath file_path;
  PlatformFileError error = GetLocalFilePath(context, url, &file_path);
  if (error != base::PLATFORM_FILE_OK)
    return error;
  return NativeFileUtil::EnsureFileExists(file_path, created);
}

PlatformFileError LocalFileUtil::CreateDirectory(
    FileSystemOperationContext* context,
    const FileSystemURL& url,
    bool exclusive,
    bool recursive) {
  base::FilePath file_path;
  PlatformFileError error = GetLocalFilePath(context, url, &file_path);
  if (error != base::PLATFORM_FILE_OK)
    return error;
  return NativeFileUtil::CreateDirectory(file_path, exclusive, recursive);
}

PlatformFileError LocalFileUtil::GetFileInfo(
    FileSystemOperationContext* context,
    const FileSystemURL& url,
    base::PlatformFileInfo* file_info,
    base::FilePath* platform_file_path) {
  base::FilePath file_path;
  PlatformFileError error = GetLocalFilePath(context, url, &file_path);
  if (error != base::PLATFORM_FILE_OK)
    return error;
  // We should not follow symbolic links in sandboxed file system.
  if (file_util::IsLink(file_path))
    return base::PLATFORM_FILE_ERROR_NOT_FOUND;
  error = NativeFileUtil::GetFileInfo(file_path, file_info);
  if (error == base::PLATFORM_FILE_OK)
    *platform_file_path = file_path;
  return error;
}

scoped_ptr<FileSystemFileUtil::AbstractFileEnumerator> LocalFileUtil::
    CreateFileEnumerator(
        FileSystemOperationContext* context,
        const FileSystemURL& root_url) {
  base::FilePath file_path;
  if (GetLocalFilePath(context, root_url, &file_path) !=
      base::PLATFORM_FILE_OK) {
    return make_scoped_ptr(new EmptyFileEnumerator)
        .PassAs<FileSystemFileUtil::AbstractFileEnumerator>();
  }
  return make_scoped_ptr(new LocalFileEnumerator(
      file_path, root_url.path(),
      file_util::FileEnumerator::FILES |
          file_util::FileEnumerator::DIRECTORIES))
      .PassAs<FileSystemFileUtil::AbstractFileEnumerator>();
}

PlatformFileError LocalFileUtil::GetLocalFilePath(
    FileSystemOperationContext* context,
    const FileSystemURL& url,
    base::FilePath* local_file_path) {
  FileSystemMountPointProvider* provider =
      context->file_system_context()->GetMountPointProvider(url.type());
  DCHECK(provider);
  base::FilePath root = provider->GetFileSystemRootPathOnFileThread(url, false);
  if (root.empty())
    return base::PLATFORM_FILE_ERROR_NOT_FOUND;
  *local_file_path = root.Append(url.path());
  return base::PLATFORM_FILE_OK;
}

PlatformFileError LocalFileUtil::Touch(
    FileSystemOperationContext* context,
    const FileSystemURL& url,
    const base::Time& last_access_time,
    const base::Time& last_modified_time) {
  base::FilePath file_path;
  PlatformFileError error = GetLocalFilePath(context, url, &file_path);
  if (error != base::PLATFORM_FILE_OK)
    return error;
  return NativeFileUtil::Touch(file_path, last_access_time, last_modified_time);
}

PlatformFileError LocalFileUtil::Truncate(
    FileSystemOperationContext* context,
    const FileSystemURL& url,
    int64 length) {
  base::FilePath file_path;
  PlatformFileError error = GetLocalFilePath(context, url, &file_path);
  if (error != base::PLATFORM_FILE_OK)
    return error;
  return NativeFileUtil::Truncate(file_path, length);
}

PlatformFileError LocalFileUtil::CopyOrMoveFile(
    FileSystemOperationContext* context,
    const FileSystemURL& src_url,
    const FileSystemURL& dest_url,
    bool copy) {
  base::FilePath src_file_path;
  PlatformFileError error = GetLocalFilePath(context, src_url, &src_file_path);
  if (error != base::PLATFORM_FILE_OK)
    return error;

  base::FilePath dest_file_path;
  error = GetLocalFilePath(context, dest_url, &dest_file_path);
  if (error != base::PLATFORM_FILE_OK)
    return error;

  return NativeFileUtil::CopyOrMoveFile(src_file_path, dest_file_path, copy);
}

PlatformFileError LocalFileUtil::CopyInForeignFile(
    FileSystemOperationContext* context,
    const base::FilePath& src_file_path,
    const FileSystemURL& dest_url) {
  if (src_file_path.empty())
    return base::PLATFORM_FILE_ERROR_INVALID_OPERATION;

  base::FilePath dest_file_path;
  PlatformFileError error =
      GetLocalFilePath(context, dest_url, &dest_file_path);
  if (error != base::PLATFORM_FILE_OK)
    return error;
  return NativeFileUtil::CopyOrMoveFile(src_file_path, dest_file_path, true);
}

PlatformFileError LocalFileUtil::DeleteFile(
    FileSystemOperationContext* context,
    const FileSystemURL& url) {
  base::FilePath file_path;
  PlatformFileError error = GetLocalFilePath(context, url, &file_path);
  if (error != base::PLATFORM_FILE_OK)
    return error;
  return NativeFileUtil::DeleteFile(file_path);
}

PlatformFileError LocalFileUtil::DeleteDirectory(
    FileSystemOperationContext* context,
    const FileSystemURL& url) {
  base::FilePath file_path;
  PlatformFileError error = GetLocalFilePath(context, url, &file_path);
  if (error != base::PLATFORM_FILE_OK)
    return error;
  return NativeFileUtil::DeleteDirectory(file_path);
}

webkit_blob::ScopedFile LocalFileUtil::CreateSnapshotFile(
    FileSystemOperationContext* context,
    const FileSystemURL& url,
    base::PlatformFileError* error,
    base::PlatformFileInfo* file_info,
    base::FilePath* platform_path) {
  DCHECK(file_info);
  // We're just returning the local file information.
  *error = GetFileInfo(context, url, file_info, platform_path);
  if (*error == base::PLATFORM_FILE_OK && file_info->is_directory)
    *error = base::PLATFORM_FILE_ERROR_NOT_A_FILE;
  return webkit_blob::ScopedFile();
}

}  // namespace fileapi
