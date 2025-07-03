#include "mmc_mem_blob.h"

namespace ock {
namespace mmc {

const StateTransTable MmcMemBlob::stateTransTable_ = BlobStateMachine::GetGlobalTransTable();

}  // namespace mmc
}  // namespace ock