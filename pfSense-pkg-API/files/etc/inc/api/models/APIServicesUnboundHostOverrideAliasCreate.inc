<?php
//   Copyright 2021 Jared Hendrickson
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

require_once("api/framework/APIModel.inc");
require_once("api/framework/APIResponse.inc");

class APIServicesUnboundHostOverrideAliasCreate extends APIModel {
    # Create our method constructor
    public function __construct() {
        parent::__construct();
        $this->privileges = ["page-all", "page-services-dnsresolver-edithost"];
        $this->change_note = "Added DNS Resolver host override alias via API";
    }

    public function action() {
        # Convert aliases config value to array if not already
        if (!is_array($this->config["unbound"]["hosts"][$this->id]["aliases"])) {
            $this->config["unbound"]["hosts"][$this->id]["aliases"] = ["item"=>[]];
        }

        $this->config["unbound"]["hosts"][$this->id]["aliases"]["item"][] = $this->validated_data;
        $this->write_config();
        mark_subsystem_dirty("unbound");
        # If user requests immediately application
        if ($this->initial_data['apply'] === true) {
            APITools\unbound_reload_config();
        }
        return APIResponse\get(0, $this->validated_data);
    }
    
    public function validate_payload() {
        # Check for our required 'id' payload value. Allow this to be bypassed internally for nested calls
        if ($this->validate_id === false) {
        } elseif (isset($this->initial_data["id"])) {
            if (array_key_exists($this->initial_data["id"], $this->config["unbound"]["hosts"])) {
                $this->id = $this->initial_data["id"];
            } else {
                $this->errors[] = APIResponse\get(2016);
            }
        } else {
            $this->errors[] = APIResponse\get(2015);
        }

        # Check for our required 'host' payload value
        if (isset($this->initial_data["host"])) {
            $this->validated_data["host"] = trim($this->initial_data["host"]);
        } else {
            $this->errors[] = APIResponse\get(2007);
        }

        # Check for our required 'domain' payload value
        if (isset($this->initial_data["domain"])) {
            $this->validated_data["domain"] = trim($this->initial_data["domain"]);
        } else {
            $this->errors[] = APIResponse\get(2008);
        }

        # Check for our required 'description' payload value
        if (isset($this->initial_data["description"])) {
            $this->validated_data["description"] = trim($this->initial_data["description"]);
        }

        # Ensure this hostname doesn't already exis
        if (APITools\is_unbound_fqdn($this->validated_data["host"], $this->validated_data["domain"])) {
            $this->errors[] = APIResponse\get(2009);
        }
    }
}