/* 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */


function validateNewUserForm() {
    	errors = 0;
	errorMsg = "";
	errorFields = "";
	if(document.createUserForm.orgName.value == "") {
	  errors++;
	  errorFields += "- No Organization selected";
	}
        if ((login = document.createUserForm.name.value) == "") {
              errors++;
              errorFields += "- No Login Name (email address)"
        } else {
              if (checkEmail(login) != 1) {
                  errors++;
                  errorFields += "- Login Name Must Be a Valid Email"
              }
        }
	if (errors > 0) {
		if (errorFields.length>0) {
		errorMsg += "The following fields were left blank.\nPlease correct and resubmit.\n\n";
		errorMsg += errorFields;
		}
		alert(errorMsg);
		return false;
	}
        return true;
}


function validateForm() {
    	errors = 0;
	errorMsg = "";
	errorFields = "";
        if (document.editUserForm.userid.value == ""){
          if ((password1 = document.editUserForm.password.value) == "") {
		if(!document.editUserForm.newPassword.checked) {
			errors++;
			errorFields += "- Password\n";
		}
          }
          if (password1 != document.editUserForm.password2.value) {
		errors++;
		errorFields += "- Passwords do not match\n";
          }
          if ((login = document.editUserForm.name.value) == "") {
              errors++;
              errorFields += "- No Login Name (email address)"
          } else {
              if (checkEmail(login) != 1) {
                  errors++;
                  errorFields += "- Login Name Must Be a Valid Email"
              }
          }
        }
	if (errors > 0) {
		if (errorFields.length>0) {
		errorMsg += "The following fields were left blank.\nPlease correct and resubmit.\n\n";
		errorMsg += errorFields;
		}
		alert(errorMsg);
		return false;
	}
        return true;
}

String.prototype.trim = function() {
	return this.replace(/^\s+|\s+$/g,"");
}

function checkEmail (strng)
{
     var filter = /^([a-zA-Z0-9_\-\+%]+\.{0,1})+@([a-zA-Z0-9\-]+\.)+[a-zA-Z]{2,4}$/;
     if(filter.test(strng))
     {
          var ind = 0;
          ind = strng.indexOf("@");
          if("." == strng.charAt(ind-1) || "." == strng.charAt(ind+1))
          {
               return 0;
          }
          else
          {
               return 1;
          }


     }
     else
     {
         return 0;
     }

}

function checkPhone(string)
{
    var filter = /^[a-zA-Z0-9\-\(\)\ ]+$/;
    if(filter.test(string))
    {
	return 1;
    } else {
	return 0;
    }
}

function checkZip(string)
{
    var filter = /(^\d{5}$)|(^\d{5}-\d{4}$)|(^([a-zA-Z]{1}\d{1}){3}$)|(^[a-zA-Z]{1}\d{1}[a-zA-Z]{1}\ \d{1}[a-zA-Z]{1}\d{1}$)/;
    if(filter.test(string))
    {
	  return 1;
    } else {
	  return 0;
    }
}
function checklatin(strn)
{

     var filter = /^[\x20-\x7A\xC0-\xFF]+$/;
	
     if(filter.test(strn))
     {
          return 1;
     }
     else
          return 0;
}

function isInteger(aNumber){
     
     var filter = /^[0-9]+$/;
     if(filter.test(aNumber)){
         return 1;
     } else {
         return 0;
     }
 }


$(document).ready(function(){
	// Bind a function to the change event of the users dropdown
	var $j = jQuery.noConflict();
        $j('select').change(function($){
            if ($j(this)[0].name == 'updateRole') {
              var accountID = $j(this)[0].value.split('|')[0];
              var newRole = $j(this)[0].value.split('|')[1];
              var aThis = $j(this);
              new Ajax.Request('/admin/updateacct.php', {parameters: {accountID: accountID, newRole: newRole},
                    onCreate: function() {
                        aThis.css({color: "red"});
                    },
                    onFailure: function() {
                        alert('Failed to update UserRole - Please try again or contact support@teleplace.com');
                    },
                    onSuccess: function() {
                         aThis.css({color: "black"}).animate({backgroundColor: "#C3FDB8"}, 1000).animate({backgroundColor: "#ffffff"},100);
                    }
              });
            } else if ($j(this)[0].name == 'updateStatus'){
                var accountID = $j(this)[0].value.split('|')[0];
              var newStatus = $j(this)[0].value.split('|')[1];
              var aThis = $j(this);
              new Ajax.Request('/admin/updatestatus.php', {parameters: {accountID: accountID, newStatus: newStatus},
                    onCreate: function() {
                        aThis.css({color: "red"});
                    },
                    onFailure: function() {
                        alert('Failed to update UserStatus - Please try again or contact support@teleplace.com');
                    },
                    onSuccess: function() {
                         aThis.css({color: "black"}).animate({backgroundColor: "#C3FDB8"}, 1000).animate({backgroundColor: "#ffffff"},100);
                    }
              });
            }
        });
});

