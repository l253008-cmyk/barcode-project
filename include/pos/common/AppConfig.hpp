// AppConfig.hpp                                      
// Settings for the POS application, all in one place (generality principle).
// To change a data path, the camera size or a scanning time, change it here only.

#ifndef POS_APP_CONFIG_HPP
#define POS_APP_CONFIG_HPP

#include <string>

// CMake sets POS_DATA_DIR to the full path of the data folder.
// Tests can set it to a temporary folder. Without CMake the program looks in ./data.
#ifndef POS_DATA_DIR
#define POS_DATA_DIR "data"
#endif

namespace pos {
	namespace config {

		// ---------- data files ----------
		inline std::string dataDir() { return POS_DATA_DIR; }
		inline std::string productFile() { return dataDir() + "/Product.txt"; }
		inline std::string orderFile() { return dataDir() + "/Order.txt"; }
		inline std::string orderDetailFile() { return dataDir() + "/OrderDetail.txt"; }
		inline std::string billFile() { return dataDir() + "/Bill.txt"; }
		inline std::string billDetailFile() { return dataDir() + "/BillDetail.txt"; }
		inline std::string categoryFile() { return dataDir() + "/PCategory.txt"; }
		inline std::string seasonFile() { return dataDir() + "/Seasons.txt"; }
		inline std::string receiptsDir() { return dataDir() + "/receipts"; }

		// ---------- security & shop info ----------
		inline std::string managerPin() { return "1234"; }
		inline std::string shopName() { return "Store POS System"; }

		// ---------- camera ----------
		// Preferred frame size. The camera format closest to this size is used.
		constexpr int kCameraWidth = 640;
		constexpr int kCameraHeight = 480;

		// ---------- scanning ----------
		constexpr int kScanIntervalMs = 150;    // try to read a frame at most once every 150 ms
		constexpr int kRepeatIgnoreMs = 2000;   // after a code is reported, ignore the same code for 2 seconds
		constexpr int kConfirmReads = 2;      // a code must be read this many times in a row to count

	}  // namespace config
}  // namespace pos

#endif  // POS_APP_CONFIG_HPP