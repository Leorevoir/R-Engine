#include "../Test.hpp"

#include <R-Engine/Core/Error.hpp>

Test(error, test_error)
{
    try {
        throw r::exception::Error("FONCTION", "MARCHE_PAS");
    } catch (const r::exception::Error &e) {
        cr_assert_str_eq(e.where(), "FONCTION");
        cr_assert_str_eq(e.what(), "MARCHE_PAS");
    }
}

Test(error, test_error_long)
{
    try {
        throw r::exception::Error("FONCTION", "MARCHE_PAS", " DU TOUT", 42);
    } catch (const r::exception::Error &e) {
        cr_assert_str_eq(e.where(), "FONCTION");
        cr_assert_str_eq(e.what(), "MARCHE_PAS DU TOUT42");
    }
}
