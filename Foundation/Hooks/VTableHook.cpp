#include "VTableHook.hpp"
#include "../Memory/MemoryUtils.hpp"
#include <cstring>

namespace Broadsword::Foundation {

std::unique_ptr<VTableHook> VTableHook::Create(void* pObject) {
  if (!pObject) {
    return nullptr;
  }

  // Get the vtable pointer (first pointer in the object)
  void** pVTable = *reinterpret_cast<void***>(pObject);
  if (!pVTable) {
    return nullptr;
  }

  // Determine vtable size by scanning until we hit non-executable memory
  size_t vtableSize = 0;
  while (true) {
    // Check if the pointer is valid and points to executable memory
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(pVTable[vtableSize], &mbi, sizeof(mbi)) == 0) {
      break;
    }

    // Check if memory is executable
    if (!(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY))) {
      break;
    }

    vtableSize++;

    // Safety limit (most vtables don't exceed 256 functions)
    if (vtableSize >= 256) {
      break;
    }
  }

  if (vtableSize == 0) {
    return nullptr;
  }

  // Use private constructor via make_unique workaround
  return std::unique_ptr<VTableHook>(new VTableHook(pObject, pVTable, vtableSize));
}

VTableHook::VTableHook(void* pObject, void** originalVTable, size_t vtableSize)
    : m_pObject(pObject), m_pOriginalVTable(originalVTable), m_vtableSize(vtableSize), m_applied(false) {
  // Allocate new vtable (copy of original)
  m_pNewVTable = new void*[m_vtableSize];
  std::memcpy(m_pNewVTable, m_pOriginalVTable, m_vtableSize * sizeof(void*));
}

VTableHook::~VTableHook() {
  Remove();

  if (m_pNewVTable) {
    delete[] m_pNewVTable;
    m_pNewVTable = nullptr;
  }
}

bool VTableHook::Hook(size_t index, void* hookFunction) {
  if (index >= m_vtableSize || !hookFunction) {
    return false;
  }

  // Store the hook
  m_hooks[index] = hookFunction;

  // If already applied, update immediately
  if (m_applied) {
    // Make vtable writable
    DWORD oldProtect;
    if (!VirtualProtect(&m_pNewVTable[index], sizeof(void*), PAGE_READWRITE, &oldProtect)) {
      return false;
    }

    // Replace function pointer
    m_pNewVTable[index] = hookFunction;

    // Restore protection
    VirtualProtect(&m_pNewVTable[index], sizeof(void*), oldProtect, &oldProtect);
  }

  return true;
}

bool VTableHook::Unhook(size_t index) {
  if (index >= m_vtableSize) {
    return false;
  }

  auto it = m_hooks.find(index);
  if (it == m_hooks.end()) {
    return false;  // Not hooked
  }

  // Remove from hooks map
  m_hooks.erase(it);

  // If applied, restore original function
  if (m_applied) {
    DWORD oldProtect;
    if (!VirtualProtect(&m_pNewVTable[index], sizeof(void*), PAGE_READWRITE, &oldProtect)) {
      return false;
    }

    m_pNewVTable[index] = m_pOriginalVTable[index];

    VirtualProtect(&m_pNewVTable[index], sizeof(void*), oldProtect, &oldProtect);
  }

  return true;
}

void* VTableHook::GetOriginal(size_t index) const {
  if (index >= m_vtableSize) {
    return nullptr;
  }

  return m_pOriginalVTable[index];
}

bool VTableHook::Apply() {
  if (m_applied) {
    return true;  // Already applied
  }

  if (m_hooks.empty()) {
    return false;  // Nothing to hook
  }

  // Update new vtable with all hook functions
  for (const auto& [index, hookFunc] : m_hooks) {
    if (index < m_vtableSize) {
      m_pNewVTable[index] = hookFunc;
    }
  }

  // Replace the object's vtable pointer
  DWORD oldProtect;
  if (!VirtualProtect(m_pObject, sizeof(void*), PAGE_READWRITE, &oldProtect)) {
    return false;
  }

  *reinterpret_cast<void***>(m_pObject) = m_pNewVTable;

  VirtualProtect(m_pObject, sizeof(void*), oldProtect, &oldProtect);

  m_applied = true;
  return true;
}

void VTableHook::Remove() {
  if (!m_applied) {
    return;
  }

  // Restore original vtable pointer
  DWORD oldProtect;
  if (VirtualProtect(m_pObject, sizeof(void*), PAGE_READWRITE, &oldProtect)) {
    *reinterpret_cast<void***>(m_pObject) = m_pOriginalVTable;
    VirtualProtect(m_pObject, sizeof(void*), oldProtect, &oldProtect);
  }

  m_applied = false;
}

bool VTableHook::IsHooked(size_t index) const {
  return m_hooks.find(index) != m_hooks.end();
}

size_t VTableHook::GetVTableSize() const {
  return m_vtableSize;
}

} // namespace Broadsword::Foundation
