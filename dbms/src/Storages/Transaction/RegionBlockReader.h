// Copyright 2022 PingCAP, Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <Columns/ColumnsNumber.h>
#include <Common/typeid_cast.h>
#include <Core/Names.h>
#include <Storages/ColumnsDescription.h>
#include <Storages/Transaction/DatumCodec.h>
#include <Storages/Transaction/DecodingStorageSchemaSnapshot.h>
#include <Storages/Transaction/Region.h>
#include <Storages/Transaction/RegionDataRead.h>
#include <Storages/Transaction/RowCodec.h>

namespace TiDB
{
struct TableInfo;
};

namespace DB
{
class IManageableStorage;
using ManageableStoragePtr = std::shared_ptr<IManageableStorage>;

struct ColumnsDescription;
class Block;

/// The Reader to read the region data in `data_list` and decode based on the given table_info and columns, as a block.
class RegionBlockReader : private boost::noncopyable
{
public:
    explicit RegionBlockReader(DecodingStorageSchemaSnapshotConstPtr schema_snapshot_);

    /// Read `data_list` as a block.
    ///
    /// On decode error, i.e. column number/type mismatch, will do force apply schema,
    /// i.e. add/remove/cast unknown/missing/type-mismatch column if force_decode is true, otherwise return empty block and false.
    /// Moreover, exception will be thrown if we see fatal decode error meanwhile `force_decode` is true.
    ///
    /// `RegionBlockReader::read` is the common routine used by both 'flush' and 'read' processes of TXN engine (Delta-Tree, TXN-MergeTree),
    /// each of which will use carefully adjusted 'force_decode' with appropriate error handling/retry to get what they want.
    bool read(Block & block, const RegionDataReadInfoList & data_list, bool force_decode);

private:
    template <TMTPKType pk_type>
    bool readImpl(Block & block, const RegionDataReadInfoList & data_list, bool force_decode);

private:
    DecodingStorageSchemaSnapshotConstPtr schema_snapshot;
};

} // namespace DB
