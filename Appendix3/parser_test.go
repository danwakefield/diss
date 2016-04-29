package arith_test

import (
	"testing"

	A "github.com/danwakefield/gosh/arith"
	"github.com/danwakefield/gosh/variables"
)

var EmptyScope = variables.NewScope()

func TestArithPrefix(t *testing.T) {
	type TestCase struct {
		in   string
		want int64
	}
	cases := []TestCase{
		{"~4", -5},
		{"~~4", 4},
		{"!1", A.ShellTrue},
		{"!4", A.ShellTrue},
		{"!0", A.ShellFalse},
		{"!!1", A.ShellFalse},
		{"1+2*3", 7},
		{"1+(2*3)", 7},
		{"(1+2)*3", 9},
	}
	for _, c := range cases {
		got, err := A.Parse(c.in, EmptyScope)
		if err != nil {
			t.Errorf("Parse returned an error: %s", err.Error())
		}
		if got != c.want {
			t.Errorf("Parse(%s) should return %d not %d", c.in, c.want, got)
		}
	}
}