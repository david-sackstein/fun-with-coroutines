#include "common/io/print.h"
#include "no-coroutines/2. parser/LineProtocolParser.h"

namespace no_coroutines {

void run_parser() {
    io::print("=== Parser protocol demo ===\n");
    io::print("Commands: SET <key> <value>, GET <key>\n\n");

    LineProtocolParser parser;
    parser.run();
}

}
