" manool.vim

if exists("b:current_syntax")| finish| endif
let s:cpo_save=&cpo| se cpo&vim

sy match mnlError display `\v[[:punct:]]`
sy match mnlSpec1 display `\v<[[:lower:]]\k*>`
sy match mnlSpec2 display `\v[!#$%&'*+./<=>?@^|~-]`
sy match mnlSpec3 display `\v_\k*>`
sy match mnlPunct display `\v[][(){}:.;]`

sy match  mnlError   display `\v<\d+\k+`
sy match  mnlNumber  display `\v<\d+>`
sy region mnlString  display start=`\v\"` end=`\v\"|$`
sy region mnlString1 display start=`\v\"` end=`\v\"|$` contained

sy region mnlComment1 display start=`\v\-\-` end=`\v$`
sy region mnlComment2 start=`\v\/\*` end=`\v\*\/` contains=mnlComment2,mnlComment1,mnlString1

sy sync fromstart
sy sync linebreaks=100

hi def link mnlSpec1    rusSpec1
hi def link mnlSpec2    rusSpec2
hi def link mnlSpec3    rusSpec3
hi def link mnlPunct    rusPunct
hi def link mnlNumber   rusNumber
hi def link mnlString   rusString
hi def link mnlString1  rusComment
hi def link mnlComment1 rusComment
hi def link mnlComment2 rusComment
hi def link mnlError    rusError

let b:current_syntax="manool"
let &cpo=s:cpo_save| unlet s:cpo_save

