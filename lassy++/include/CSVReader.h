/*
*   Source: https://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c
*/
#include <istream>
#include <string>
#include <vector>
;


class CSVReader
{   
   
    public:
        
        enum CSVState {
            UnquotedField,
            QuotedField,
            QuotedQuote
        };

        static std::vector<std::string> readCSVRow(const std::string &row) {
            CSVState state = CSVState::UnquotedField;
            const char *vinit[] = {""};
            std::vector<std::string> fields(vinit, std::end(vinit));   
            size_t i = 0; // index of the current field
            for (char c : row) {
                switch (state) {
                    case CSVState::UnquotedField:
                        switch (c) {
                            case ',': // end of field
                                    fields.push_back(""); i++;
                                    break;
                            case '"': state = CSVState::QuotedField;
                                    break;
                            default:  fields[i].push_back(c);
                                    break; 
                        }
                        break;
                    case CSVState::QuotedField:
                        switch (c) {
                            case '"': state = CSVState::QuotedQuote;
                                    break;
                            default:  fields[i].push_back(c);
                                    break; 
                        }
                        break;
                    case CSVState::QuotedQuote:
                        switch (c) {
                            case ',': // , after closing quote
                                    fields.push_back(""); i++;
                                    state = CSVState::UnquotedField;
                                    break;
                            case '"': // "" -> "
                                    fields[i].push_back('"');
                                    state = CSVState::QuotedField;
                                    break;
                            default:  // end of quote
                                    state = CSVState::UnquotedField;
                                    break; 
                        }
                        break;
                } // end switch 
            }
            return fields;
        } // end function

    /*
    * An example usage can be found in LaImage::FileToPixel() 
    */
        static  std::vector<std::vector<std::string> > readCSV(std::istream &in) {
            
            std::vector<std::vector<std::string> > table;
            std::string row;
            while (!in.eof()) {
                std::getline(in, row);
                if (in.bad() || in.fail()) {
                    break;
                }
                auto fields = CSVReader::readCSVRow(row);
                table.push_back(fields);
            }
            return table;
        }
    
};      // end class